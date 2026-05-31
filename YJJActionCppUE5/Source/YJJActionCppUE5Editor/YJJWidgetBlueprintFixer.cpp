#include "YJJWidgetBlueprintFixer.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "BlueprintEditorLibrary.h"
#include "Components/Widget.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "UObject/UObjectGlobals.h"
#include "WidgetBlueprint.h"

namespace
{
	struct FYjjWidgetRepairEntry
	{
		const TCHAR* AssetPath;
		const TCHAR* ParentClassSuffix;
	};

	// 자식 → 부모 순 (리프 먼저, HUD 루트 마지막)
	const FYjjWidgetRepairEntry GWidgetRepairs[] = {
		{ TEXT("/Game/Widgets/Player/CWB_Player_Bar"), TEXT("PlayerBar") },
		{ TEXT("/Game/Widgets/Player/CWB_Player_Level"), TEXT("PlayerLevel") },
		{ TEXT("/Game/Widgets/Player/WB_Player_HpBar"), TEXT("PlayerBar") },
		{ TEXT("/Game/Widgets/Player/WB_Player_ManaBar"), TEXT("PlayerBar") },
		{ TEXT("/Game/Widgets/Player/WB_Player_StaminaBar"), TEXT("PlayerBar") },
		{ TEXT("/Game/Widgets/Player/WB_Player_Level"), TEXT("PlayerLevel") },
		{ TEXT("/Game/Widgets/Enemy/CWB_Enemy_HpBar_Guage"), TEXT("EnemyBar") },
		{ TEXT("/Game/Widgets/Enemy/WB_Enemy_HpBar_Guage"), TEXT("EnemyBar") },
		{ TEXT("/Game/Widgets/Boss/WB_Boss_GroggyBar_Guage"), TEXT("BossGroggyBarGauge") },
		{ TEXT("/Game/Widgets/Boss/WB_Boss_GroggyBar"), TEXT("BossGroggyBar") },
		{ TEXT("/Game/Widgets/Weapons/CWB_EquipMenuButton"), TEXT("EquipMenuButton") },
		{ TEXT("/Game/Widgets/Weapons/WB_EquipMenuButton"), TEXT("EquipMenuButton") },
		{ TEXT("/Game/Widgets/Weapons/CWB_EquipMenu"), TEXT("EquipMenu") },
		{ TEXT("/Game/Widgets/Weapons/WB_EquipMenu"), TEXT("EquipMenu") },
		{ TEXT("/Game/Widgets/Weapons/WB_MagicMenu"), TEXT("MagicMenu") },
		{ TEXT("/Game/Widgets/Weapons/WB_MagicMenuButton"), TEXT("EquipMenuButton") },
		{ TEXT("/Game/Widgets/Interaction/CWB_Interaction"), TEXT("Interaction") },
		{ TEXT("/Game/Widgets/Interaction/WB_Interaction"), TEXT("Interaction") },
		{ TEXT("/Game/Widgets/Interaction/CWB_Targeting"), TEXT("Custom") },
		{ TEXT("/Game/Widgets/Enemy/WB_Targeting"), TEXT("Targeting") },
		{ TEXT("/Game/Widgets/Player/CWB_Player_Info"), TEXT("PlayerInfo") },
		{ TEXT("/Game/Widgets/Player/WB_Player_Info"), TEXT("PlayerInfo") },
		{ TEXT("/Game/Widgets/Boss/WB_Boss_Info"), TEXT("Custom") },
		{ TEXT("/Game/Widgets/WB_HUDUI"), TEXT("HUD") },
		{ TEXT("/Game/Widgets/CWB_HUD"), TEXT("HUD") },
	};
}

void UYJJWidgetBlueprintFixer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// PostEngineInit 직후 한 틱 미룸 — YJJActionCppUE5 런타임 모듈·리다이렉트가 모두 올라온 뒤 실행.
	if (GEditor)
	{
		GEditor->GetTimerManager()->SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UYJJWidgetBlueprintFixer::RunStartupFix));
	}
}

void UYJJWidgetBlueprintFixer::RunStartupFix()
{
	if (GIsPlayInEditorWorld || IsRunningCommandlet())
		return;

	UE_LOG(LogTemp, Log, TEXT("[YJJ WidgetFix] 에디터 기동 — Widget BP 일괄 Reparent/GUID 정리 시작"));
	FixAllWidgetBlueprints();
	UE_LOG(LogTemp, Log, TEXT("[YJJ WidgetFix] 에디터 기동 — Widget BP 일괄 처리 완료"));
}

UClass* UYJJWidgetBlueprintFixer::LoadWidgetParentClass(const TCHAR* ClassSuffix)
{
	const FString PathWithU = FString::Printf(TEXT("/Script/YJJActionCppUE5.UCUserWidget_%s"), ClassSuffix);
	const FString PathWithC = FString::Printf(TEXT("/Script/YJJActionCppUE5.CUserWidget_%s"), ClassSuffix);

	const FString CandidatePaths[] = { PathWithU, PathWithC };
	for (const FString& Path : CandidatePaths)
	{
		UClass* LoadedClass = FindObject<UClass>(nullptr, *Path);
		if (IsValid(LoadedClass))
			return LoadedClass;

		LoadedClass = LoadObject<UClass>(nullptr, *Path);
		if (IsValid(LoadedClass))
			return LoadedClass;
	}

	UE_LOG(LogTemp, Error, TEXT("[YJJ WidgetFix] 부모 C++ 클래스 로드 실패 — %s (리빌드 확인)"), ClassSuffix);
	return nullptr;
}

int32 UYJJWidgetBlueprintFixer::PruneStaleWidgetVariableGuids(UWidgetBlueprint* WidgetBlueprint)
{
	if (false == IsValid(WidgetBlueprint))
		return 0;

	TSet<FName> SeenVariableNames;
	WidgetBlueprint->ForEachSourceWidget([&SeenVariableNames](UWidget* Widget)
	{
		if (IsValid(Widget))
			SeenVariableNames.Add(Widget->GetFName());
	});

	for (UWidgetAnimation* Animation : WidgetBlueprint->Animations)
	{
		if (IsValid(Animation))
			SeenVariableNames.Add(Animation->GetFName());
	}

	int32 RemovedCount = 0;
	TArray<FName> StaleKeys;
	for (const TPair<FName, FGuid>& Pair : WidgetBlueprint->WidgetVariableNameToGuidMap)
	{
		if (false == SeenVariableNames.Contains(Pair.Key))
			StaleKeys.Add(Pair.Key);
	}

	for (const FName& StaleKey : StaleKeys)
	{
		WidgetBlueprint->WidgetVariableNameToGuidMap.Remove(StaleKey);
		RemovedCount++;
		UE_LOG(LogTemp, Warning, TEXT("[YJJ WidgetFix] orphan GUID 제거 — %s.%s"),
			*WidgetBlueprint->GetName(), *StaleKey.ToString());
	}

	return RemovedCount;
}

bool UYJJWidgetBlueprintFixer::ReparentAndSaveWidgetBlueprint(const FSoftObjectPath& AssetPath, UClass* NewParentClass)
{
	if (false == AssetPath.IsValid() || false == IsValid(NewParentClass))
		return false;

	UWidgetBlueprint* WidgetBlueprint = LoadObject<UWidgetBlueprint>(nullptr, *AssetPath.ToString());
	if (false == IsValid(WidgetBlueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("[YJJ WidgetFix] SKIP (없음): %s"), *AssetPath.ToString());
		return false;
	}

	const bool bNeedsReparent = WidgetBlueprint->ParentClass != NewParentClass;
	const int32 PrunedGuidCount = PruneStaleWidgetVariableGuids(WidgetBlueprint);

	if (bNeedsReparent)
	{
		UE_LOG(LogTemp, Log, TEXT("[YJJ WidgetFix] Reparent: %s -> %s"),
			*AssetPath.ToString(), *NewParentClass->GetName());
		UBlueprintEditorLibrary::ReparentBlueprint(WidgetBlueprint, NewParentClass);
	}

	if (false == bNeedsReparent && PrunedGuidCount <= 0)
		return true;

	PruneStaleWidgetVariableGuids(WidgetBlueprint);

	FBlueprintEditorUtils::RefreshAllNodes(WidgetBlueprint);
	const EBlueprintCompileOptions CompileOptions = EBlueprintCompileOptions::SkipSave;
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint, CompileOptions);

	WidgetBlueprint->MarkPackageDirty();
	const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages(
		{ WidgetBlueprint->GetOutermost() },
		false);

	if (false == bSaved)
	{
		UE_LOG(LogTemp, Error, TEXT("[YJJ WidgetFix] SAVE FAILED: %s"), *AssetPath.ToString());
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("[YJJ WidgetFix] DONE: %s (reparent=%d pruned=%d)"),
		*AssetPath.ToString(), bNeedsReparent ? 1 : 0, PrunedGuidCount);
	return true;
}

void UYJJWidgetBlueprintFixer::FixAllWidgetBlueprints()
{
	int32 SuccessCount = 0;
	int32 FailCount = 0;

	for (const FYjjWidgetRepairEntry& Entry : GWidgetRepairs)
	{
		UClass* ParentClass = LoadWidgetParentClass(Entry.ParentClassSuffix);
		if (false == IsValid(ParentClass))
		{
			FailCount++;
			continue;
		}

		if (ReparentAndSaveWidgetBlueprint(FSoftObjectPath(Entry.AssetPath), ParentClass))
			SuccessCount++;
		else
			FailCount++;
	}

	UE_LOG(LogTemp, Log, TEXT("[YJJ WidgetFix] 결과 — success=%d failed=%d"), SuccessCount, FailCount);
}
