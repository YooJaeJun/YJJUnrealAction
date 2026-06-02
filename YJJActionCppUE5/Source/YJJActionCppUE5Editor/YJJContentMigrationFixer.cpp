#include "YJJContentMigrationFixer.h"

#include "Editor.h"
#include "Engine/DataTable.h"
#include "FileHelpers.h"
#include "Parkour/FParkourData.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "Weapons/CWeaponAsset.h"

namespace
{
	const TCHAR* const GWeaponDataAssetPaths[] = {
		TEXT("/Game/Weapons/Fist/CDA_Fist.CDA_Fist"),
		TEXT("/Game/Weapons/Sword/CDA_Sword.CDA_Sword"),
		TEXT("/Game/Weapons/Hammer/CDA_Hammer.CDA_Hammer"),
		TEXT("/Game/Weapons/Dual/CDA_Dual.CDA_Dual"),
		TEXT("/Game/Weapons/Bow/CDA_Bow.CDA_Bow"),
		TEXT("/Game/Weapons/Warp/CDA_Warp.CDA_Warp"),
		TEXT("/Game/Weapons/Around/CDA_Around.CDA_Around"),
		TEXT("/Game/Weapons/Fireball/CDA_Fireball.CDA_Fireball"),
		TEXT("/Game/Weapons/Bomb/CDA_Bomb.CDA_Bomb"),
		TEXT("/Game/Weapons/Yondu/CDA_Yondu.CDA_Yondu"),
	};
}

void UYJJContentMigrationFixer::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// WidgetFixer 와 동일 — Blueprint Validator 등록 이후에 실행해야 Save/Validate 스팸이 없다.
	if (GEditor)
	{
		FEditorDelegates::OnEditorInitialized.AddUObject(this, &UYJJContentMigrationFixer::ScheduleStartupMigration);
	}
}

void UYJJContentMigrationFixer::ScheduleStartupMigration(double InDuration)
{
	(void)InDuration;
	FEditorDelegates::OnEditorInitialized.RemoveAll(this);

	if (GEditor)
	{
		GEditor->GetTimerManager()->SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UYJJContentMigrationFixer::RunStartupMigration));
	}
}

void UYJJContentMigrationFixer::RunStartupMigration()
{
	if (GIsPlayInEditorWorld || IsRunningCommandlet())
		return;

	UE_LOG(LogTemp, Log, TEXT("[YJJ ContentMigration] CDA / ParkourData 마이그레이션 시작"));
	ResaveWeaponDataAssets();
	FixParkourDataTableRowStruct();
	UE_LOG(LogTemp, Log, TEXT("[YJJ ContentMigration] CDA / ParkourData 마이그레이션 완료"));
}

bool UYJJContentMigrationFixer::SavePackageForObject(UObject* Object)
{
	if (false == IsValid(Object))
		return false;

	UPackage* const package = Object->GetOutermost();
	if (false == IsValid(package))
		return false;

	Object->Modify();
	package->MarkPackageDirty();
	return UEditorLoadingAndSavingUtils::SavePackages({ package }, false);
}

void UYJJContentMigrationFixer::ResaveWeaponDataAssets()
{
	int32 savedCount = 0;
	int32 failedCount = 0;

	for (const TCHAR* const assetPath : GWeaponDataAssetPaths)
	{
		UObject* const loadedObject = StaticLoadObject(UObject::StaticClass(), nullptr, assetPath);
		UCWeaponAsset* const weaponAsset = Cast<UCWeaponAsset>(loadedObject);
		if (false == IsValid(weaponAsset))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[YJJ ContentMigration] CDA 로드/캐스트 실패 — %s (CoreRedirects CWeaponAsset 확인)"),
				assetPath);
			++failedCount;
			continue;
		}

		if (SavePackageForObject(weaponAsset))
		{
			++savedCount;
			UE_LOG(LogTemp, Log, TEXT("[YJJ ContentMigration] CDA 저장 — %s"), assetPath);
		}
		else
		{
			++failedCount;
			UE_LOG(LogTemp, Error, TEXT("[YJJ ContentMigration] CDA 저장 실패 — %s"), assetPath);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[YJJ ContentMigration] CDA 결과 — saved=%d failed=%d"), savedCount, failedCount);
}

void UYJJContentMigrationFixer::FixParkourDataTableRowStruct()
{
	UDataTable* const dataTable = LoadObject<UDataTable>(
		nullptr,
		TEXT("/Game/Parkour/ParkourData.ParkourData"));
	if (false == IsValid(dataTable))
	{
		UE_LOG(LogTemp, Warning, TEXT("[YJJ ContentMigration] ParkourData DT 없음 — /Game/Parkour/ParkourData"));
		return;
	}

	UScriptStruct* const expectedStruct = FParkourData::StaticStruct();
	const UScriptStruct* const currentStruct = dataTable->GetRowStruct();
	if (currentStruct == expectedStruct)
		return;

	const int32 rowCount = dataTable->GetRowNames().Num();
	if (rowCount > 0)
	{
		// RowStruct 만 바꾼 채 기존 행을 Save 하면 직렬화 중 FObjectPtr 해시 크래시가 난다 — 에디터 수동 마이그레이션 필요.
		UE_LOG(LogTemp, Warning,
			TEXT("[YJJ ContentMigration] ParkourData RowStruct 불일치 — 행 %d개 있어 자동 변경 생략. ")
			TEXT("에디터: ParkourData DT → Row Structure=YJJActionCppUE5.FParkourData 로 바꾼 뒤 행을 다시 입력(또는 CSV 재임포트). ")
			TEXT("현재=%s"),
			rowCount,
			IsValid(currentStruct) ? *currentStruct->GetStructPathName().ToString() : TEXT("(null)"));
		return;
	}

	dataTable->RowStruct = expectedStruct;
	if (SavePackageForObject(dataTable))
	{
		UE_LOG(LogTemp, Log,
			TEXT("[YJJ ContentMigration] ParkourData RowStruct → YJJActionCppUE5.FParkourData 저장 완료 (빈 테이블)"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[YJJ ContentMigration] ParkourData 저장 실패"));
	}
}
