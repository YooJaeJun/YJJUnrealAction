#include "Commons/CYJJBlueprintLibrary.h"
#include "Utilities/YJJLocalizedText.h"
#include "Utilities/CLog.h"
#include "Internationalization/Text.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"
#include "Characters/CCommonCharacter.h"
#include "Components/PrimitiveComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetSystemLibrary.h"
#include <cmath>

FText UCYJJBlueprintLibrary::GetLocalizedUI(FName Key)
{
	return YJJLocalization::LocalizedText_From_UserInterface_Localization_Table_Key(Key);
}

void UCYJJBlueprintLibrary::BreakCCharacterInfo(
	const FCCharacterInfo& CharacterInfo,
	int32& Type,
	int32& Group,
	FName& Name,
	FLinearColor& BodyColor)
{
	Type = CharacterInfo.Type;
	Group = CharacterInfo.Group;
	Name = CharacterInfo.Name;
	BodyColor = CharacterInfo.BodyColor;
}

void UCYJJBlueprintLibrary::BreakHitData(
	const FHitData& HitData,
	UAnimMontage*& Montage,
	float& PlayRate,
	float& Damage,
	CEAttackType& AttackType,
	bool& bCanMove,
	float& Launch,
	CECrowdControl& CrowdControl,
	float& HitStop,
	USoundBase*& Sound,
	UFXSystemAsset*& Effect,
	FVector& EffectLocation,
	FVector& EffectScale,
	TSubclassOf<UCameraShakeBase>& ShakeClass)
{
	Montage = HitData.Montage;
	PlayRate = HitData.PlayRate;
	Damage = HitData.Damage;
	AttackType = HitData.AttackType;
	bCanMove = HitData.bCanMove;
	Launch = HitData.Launch;
	CrowdControl = HitData.CrowdControl;
	HitStop = HitData.HitStop;
	Sound = HitData.Sound;
	Effect = HitData.Effect;
	EffectLocation = HitData.EffectLocation;
	EffectScale = HitData.EffectScale;
	ShakeClass = HitData.ShakeClass;
}

bool UCYJJBlueprintLibrary::AreCharactersSameGroup(ACCommonCharacter* InA, ACCommonCharacter* InB)
{
	// 둘 중 하나라도 무효면 동일 그룹 여부 없음으로 false.
	if (false == IsValid(InA) || false == IsValid(InB))
		return false;

	return InA->CharacterInfo.Group == InB->CharacterInfo.Group;
}

bool UCYJJBlueprintLibrary::TryAddUniqueCommonCharacter(
	TArray<ACCommonCharacter*>& InOutCharacters,
	ACCommonCharacter* ToAddCandidate)
{
	if (false == IsValid(ToAddCandidate))
		return false;

	if (InOutCharacters.Contains(ToAddCandidate))
		return false;

	InOutCharacters.Add(ToAddCandidate);
	return true;
}

ACharacter* UCYJJBlueprintLibrary::FindNearlyFrontCharacter(
	ACharacter* ReferenceCharacter,
	const TArray<ACharacter*>& CandidateCharacters,
	double LimitMinimumDot)
{
	if (false == IsValid(ReferenceCharacter))
		return nullptr;

	AController* controller = nullptr;
	ACCommonCharacter* refCommon = Cast<ACCommonCharacter>(ReferenceCharacter);
	if (IsValid(refCommon))
		controller = refCommon->GetMyCurController().Get();

	if (false == IsValid(controller))
		controller = ReferenceCharacter->GetController();

	if (false == IsValid(controller))
		return nullptr;

	const FRotator controlRot = controller->GetControlRotation();
	const FRotator yawOnlyLocal(0.0, controlRot.Yaw, 0.0);
	FVector forwardLocal = yawOnlyLocal.Vector();
	forwardLocal.Z = 0.0;
	if (false == forwardLocal.Normalize())
		return nullptr;

	const FVector referenceLocationLocal = ReferenceCharacter->GetActorLocation();

	ACharacter* bestCharacterLocal = nullptr;
	double bestDotLocal = -2.0;

	const int32 candidateCountLocal = CandidateCharacters.Num();
	for (int32 candidateIndexLocal = 0; candidateIndexLocal < candidateCountLocal; ++candidateIndexLocal)
	{
		ACharacter* candidateLocal = CandidateCharacters[candidateIndexLocal];
		if (false == IsValid(candidateLocal))
			continue;

		FVector deltaLocal = candidateLocal->GetActorLocation() - referenceLocationLocal;
		deltaLocal.Z = 0.0;
		if (false == deltaLocal.Normalize())
			continue;

		const double dotLocal = static_cast<double>(FVector::DotProduct(deltaLocal, forwardLocal));
		if (dotLocal <= LimitMinimumDot)
			continue;
		if (dotLocal > bestDotLocal)
		{
			bestDotLocal = dotLocal;
			bestCharacterLocal = candidateLocal;
		}
	}

	return bestCharacterLocal;
}

double UCYJJBlueprintLibrary::RotateYawFrom360ToMinus180Degrees(const double DegreesAroundFullCircle)
{
	const double summedLocal = DegreesAroundFullCircle + 180.0;
	const double remainderLocal = std::fmod(summedLocal, 360.0);
	return remainderLocal - 180.0;
}

void UCYJJBlueprintLibrary::SetStateRenderText(UTextRenderComponent* RenderComponent, const FText& DisplayText)
{
	if (false == IsValid(RenderComponent))
	{
		CLog::Log(TEXT("SetStateRenderText: RenderComponent 가 nullptr 이면 무시된다."));
		return;
	}

	RenderComponent->SetText(DisplayText);
}

bool UCYJJBlueprintLibrary::TryGetHitPointBetweenPrimitives(
	UPrimitiveComponent* OverlapStartComponent,
	UPrimitiveComponent* OtherComponent,
	float SphereRadius,
	const TArray<AActor*>& ActorsToIgnore,
	FVector& OutImpactPoint)
{
	if (false == IsValid(OverlapStartComponent) || false == IsValid(OtherComponent))
		return false;

	UWorld* worldLocal = OverlapStartComponent->GetWorld();
	if (false == IsValid(worldLocal))
		return false;

	const FVector startLocal = OverlapStartComponent->K2_GetComponentLocation();
	const FVector endLocal = OtherComponent->K2_GetComponentLocation();

	TArray<TEnumAsByte<EObjectTypeQuery>> objectTypesLocal;
	objectTypesLocal.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	objectTypesLocal.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

	TArray<FHitResult> hitsLocal;
	const bool traceHitSomethingLocal = UKismetSystemLibrary::SphereTraceMultiForObjects(
		worldLocal,
		startLocal,
		endLocal,
		SphereRadius,
		objectTypesLocal,
		false,
		ActorsToIgnore,
		EDrawDebugTrace::None,
		hitsLocal,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		5.0f);

	if (false == traceHitSomethingLocal || hitsLocal.Num() <= 0)
		return false;

	OutImpactPoint = hitsLocal[0].ImpactPoint;
	return true;
}
