#include "Commons/CYJJBlueprintLibrary.h"
#include "Utilities/YJJLocalizedText.h"
#include "Utilities/CLog.h"
#include "Internationalization/Text.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Animals/CAnimal.h"
#include "Characters/Animals/CAnimalWeapon.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Components/CStateComponent.h"
#include "Components/CPatrolComponent.h"
#include "GameFramework/Pawn.h"
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

bool UCYJJBlueprintLibrary::BFCompat_IsSameGroup(ACCommonCharacter* InCharacterA, ACCommonCharacter* InCharacterB)
{
	return AreCharactersSameGroup(InCharacterA, InCharacterB);
}

bool UCYJJBlueprintLibrary::BFCompat_IsIdleFromControlledPawn(APawn* ControlledPawn)
{
	// 무효 Pawn / CommonCharacter 계열이 아닌 Pawn → idle 판별 불가(행트리에서 해당 경로 진입 불가하면 정상).
	if (false == IsValid(ControlledPawn))
		return false;

	ACCommonCharacter* const commonCharacter = Cast<ACCommonCharacter>(ControlledPawn);
	if (false == IsValid(commonCharacter))
		return false;

	return commonCharacter->IsIdle();
}

bool UCYJJBlueprintLibrary::BFCompat_GetAnimalWeaponInAction(ACAnimalWeapon* AnimalWeapon)
{
	// 무기 없음 또는 스폰 직후 경합 시 false 로 충분(행 트리 틱에서 자주 접근 가능 → 로그 생략).
	if (false == IsValid(AnimalWeapon))
		return false;

	return AnimalWeapon->InAction;
}

ACAnimalWeapon* UCYJJBlueprintLibrary::BFCompat_GetAnimalWeaponFromControlledPawn(APawn* ControlledPawn)
{
	// 제거된 BP_Animal_AI.Weapon 대신 ACAnimal::AnimalWeapon — 비동물 Pawn/무기 미스폰은 BT 경로에서 흔함, 틱 루프 로그 생략.
	if (false == IsValid(ControlledPawn))
		return nullptr;

	ACAnimal* const Animal = Cast<ACAnimal>(ControlledPawn);
	if (false == IsValid(Animal))
		return nullptr;

	AActor* const SpawnedWeapon = Animal->AnimalWeapon.Get();
	if (false == IsValid(SpawnedWeapon))
		return nullptr;

	return Cast<ACAnimalWeapon>(SpawnedWeapon);
}

ACDragon* UCYJJBlueprintLibrary::BFCompat_GetControlledDragon(APawn* ControlledPawn)
{
	// 플라이 패스 블피 당일 다수 접근 가능 — 비드래곤 Pawn 에서 호출되는 것만 자연스레 nullptr.
	if (false == IsValid(ControlledPawn))
		return nullptr;

	return Cast<ACDragon>(ControlledPawn);
}

bool UCYJJBlueprintLibrary::BFCompat_GetDragonWeaponInAction(ACDragon* Dragon)
{
	if (false == IsValid(Dragon))
		return false;

	AActor* const WeaponActor = Dragon->DragonWeapon.Get();
	if (false == IsValid(WeaponActor))
		return false;

	return BFCompat_GetAnimalWeaponInAction(Cast<ACAnimalWeapon>(WeaponActor));
}

bool UCYJJBlueprintLibrary::BFCompat_IsDragonBossWeaponsOrAnimalInAction(APawn* ControlledPawn)
{
	ACDragon* const dragonLocal = BFCompat_GetControlledDragon(ControlledPawn);

	if ((true == IsValid(dragonLocal)) && (true == BFCompat_GetDragonWeaponInAction(dragonLocal)))
		return true;

	ACAnimalWeapon* const genericAnimalWeaponLocal = BFCompat_GetAnimalWeaponFromControlledPawn(ControlledPawn);

	return BFCompat_GetAnimalWeaponInAction(genericAnimalWeaponLocal);
}

UCStateComponent* UCYJJBlueprintLibrary::BFCompat_FindStateComponentOnPawn(APawn* ControlledPawn)
{
	// 상태 컴포넌트 없는 Pawn — 행동 트리 틱·초기화에서 빈번 호출될 수 있어 로그 없이 nullptr(호출 그래프에서 무시 또는 실패 처리).
	if (false == IsValid(ControlledPawn))
		return nullptr;

	return ControlledPawn->FindComponentByClass<UCStateComponent>();
}

CEStateType UCYJJBlueprintLibrary::BFCompat_GetStateTypeFromControlledPawn(APawn* ControlledPawn)
{
	// BT 틱에서 자주 호출 — 컴포넌트 없음은 Idle 근사로 조용히 처리(비적 AI Pawn 또는 스폰 직후).
	if (false == IsValid(ControlledPawn))
		return CEStateType::Idle;

	UCStateComponent* const state = BFCompat_FindStateComponentOnPawn(ControlledPawn);
	if (false == IsValid(state))
		return CEStateType::Idle;

	return state->GetCurMode();
}

ACCommonCharacter* UCYJJBlueprintLibrary::BFCompat_GetCommonCharacterFromControlledPawn(APawn* ControlledPawn)
{
	if (false == IsValid(ControlledPawn))
		return nullptr;

	return Cast<ACCommonCharacter>(ControlledPawn);
}

UCPatrolComponent* UCYJJBlueprintLibrary::BFCompat_FindPatrolComponentOnPawn(APawn* ControlledPawn)
{
	// Pawn 또는 PatrolComp 미부착 캐릭터에서는 nullptr — 순찰 BT 의 랜덤 내비 분기 등으로 처리(티크·실행 빈번 → 로그 없음).
	if (false == IsValid(ControlledPawn))
		return nullptr;

	return ControlledPawn->FindComponentByClass<UCPatrolComponent>();
}

void UCYJJBlueprintLibrary::BFCompat_PatrolGetMoveTo(APawn* ControlledPawn, bool& OutResult, FVector& OutLocation)
{
	OutResult = false;
	OutLocation = FVector::ZeroVector;

	UCPatrolComponent* const patrol = BFCompat_FindPatrolComponentOnPawn(ControlledPawn);
	if (false == IsValid(patrol))
		return;

	patrol->GetMoveTo(OutResult, OutLocation);
}

void UCYJJBlueprintLibrary::BFCompat_PatrolUpdateNextIndex(APawn* ControlledPawn)
{
	UCPatrolComponent* const patrol = BFCompat_FindPatrolComponentOnPawn(ControlledPawn);
	if (false == IsValid(patrol))
		return;

	patrol->UpdateNextIndex();
}

bool UCYJJBlueprintLibrary::BFCompat_PatrolIsValidPath(APawn* ControlledPawn)
{
	UCPatrolComponent* const patrol = BFCompat_FindPatrolComponentOnPawn(ControlledPawn);
	if (false == IsValid(patrol))
		return false;

	return patrol->IsValidPath();
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
