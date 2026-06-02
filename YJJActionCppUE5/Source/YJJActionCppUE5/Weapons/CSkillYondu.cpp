#include "Weapons/CSkillYondu.h"



#include "Characters/CCommonCharacter.h"

#include "Commons/CYJJBlueprintLibrary.h"

#include "Components/CCharacterStatComponent.h"

#include "Components/CapsuleComponent.h"

#include "Components/PrimitiveComponent.h"

#include "Components/SplineComponent.h"

#include "Engine/HitResult.h"

#include "Engine/World.h"

#include "GameFramework/Actor.h"

#include "Kismet/KismetSystemLibrary.h"

#include "NiagaraComponent.h"

#include "Particles/ParticleSystemComponent.h"

#include "TimerManager.h"

#include "Utilities/CLog.h"

#include "Utilities/YJJHelpers.h"

#include "Weapons/CSkillWeapon.h"



namespace

{

	static const TCHAR* const YonduNiagaraAssetPathScratch = TEXT(

		"NiagaraSystem'/Game/Effects/Yondu/NS_Yondu.NS_Yondu'");

	static const TCHAR* const YonduCascadeProjectilePathScratch = TEXT(

		"ParticleSystem'/Game/Assets/Effects/ArcherySystem/Assets/FX/Particles/Core/"

		"P_SingleTargetCore_Projectile.P_SingleTargetCore_Projectile'");



	/** 블프 `MoveComponentTo` 의 `bEaseIn=true` 근사(가속 시작). 틱 경로만 사용. */

	float SkillYondu_EaseInCubicScratch(float const AlphaScratch)

	{

		return AlphaScratch * AlphaScratch * AlphaScratch;

	}

} // namespace



ACSkillYondu::ACSkillYondu()

{

	PrimaryActorTick.bCanEverTick = true;



	bReplicates = true;

	SetReplicateMovement(true);



	ArrotRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ArrotRoot"));

	ArrotRoot->SetupAttachment(DefaultSceneRoot);

	ArrotRoot->SetRelativeLocation(FVector(-20.4f, 0.f, 0.f));



	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));

	Spline->SetupAttachment(DefaultSceneRoot);

	Spline->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.6f));



	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));

	Capsule->SetupAttachment(ArrotRoot);

	Capsule->SetCapsuleHalfHeight(80.f);

	Capsule->SetRelativeLocation(FVector(10.f, 0.f, 0.f));

	Capsule->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));



	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);



	Capsule->SetGenerateOverlapEvents(true);

	// 레거시 블프는 Capsule 에 NavArea_Obstacle 을 지정했다. 엔진 include 경로가 환경마다 달라 C++ 디폴트는 비워 두고 블프/서브클래스에서 재지정 가능.



	YonduNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NS_Yondu"));

	YonduNiagara->SetupAttachment(ArrotRoot);

	YonduNiagara->SetRelativeLocation(FVector(0.f, 0.f, -58.f));

	YonduNiagara->SetRelativeRotation(FRotator(-90.f, -85.f, -94.f));



	YonduCascade = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Cascade_ProjectileCore"));

	YonduCascade->SetupAttachment(ArrotRoot);

	YonduCascade->SetRelativeLocation(FVector(0.f, 0.f, -10.f));

	YonduCascade->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

	YonduCascade->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));

	YonduCascade->PrimaryComponentTick.bStartWithTickEnabled = false;

	YonduCascade->SetAutoActivate(false);



	SkillYondu_BindSoftAssetsIfUnsetScratch();

}



void ACSkillYondu::BeginPlay()

{

	SkillYondu_BindSoftAssetsIfUnsetScratch();



	Super::BeginPlay();



	if ((true == IsValid(YonduNiagara.Get())) && (true == IsValid(YonduNiagaraSystemAsset.Get())))

	{

		YonduNiagara->SetAsset(YonduNiagaraSystemAsset.Get());

	}

	if ((true == IsValid(YonduCascade.Get())) && (true == IsValid(YonduCascadeProjectileAsset.Get())))

	{

		YonduCascade->SetTemplate(YonduCascadeProjectileAsset.Get());

	}



	if (true == IsValid(Capsule.Get()))

	{

		Capsule->OnComponentBeginOverlap.AddDynamic(this, &ACSkillYondu::SkillYondu_OnCapsuleBeginOverlap);

	}



	SkillYondu_UpdateCapsuleHitCollisionScratch();

}



void ACSkillYondu::EndPlay(const EEndPlayReason::Type EndPlayReason)

{

	UWorld* WorldScratch = GetWorld();

	if (true == IsValid(WorldScratch))

	{

		WorldScratch->GetTimerManager().ClearTimer(SkillYondu_LifeTimeTimerHandleScratch);

	}



	Super::EndPlay(EndPlayReason);

}



bool ACSkillYondu::SkillYondu_ShouldRunCosmeticsScratch() const

{

	UWorld const* WorldScratch = GetWorld();



	if (false == IsValid(WorldScratch))

	{

		return false;

	}



	const ENetMode NetModeScratch = WorldScratch->GetNetMode();



	return NM_DedicatedServer != NetModeScratch;

}



void ACSkillYondu::SkillYondu_RebuildActorsToIgnoreScratch()

{

	ActorsToIgnore.Reset();



	ActorsToIgnore.AddUnique(this);



	ACCommonCharacter* CharacterScratch = Character.Get();

	if (true == IsValid(CharacterScratch))

	{

		ActorsToIgnore.AddUnique(CharacterScratch);



		if (true == IsValid(CharacterScratch->CurInteractingActor))

		{

			ActorsToIgnore.AddUnique(CharacterScratch->CurInteractingActor.Get());

		}

	}

}



void ACSkillYondu::SkillYondu_BindSoftAssetsIfUnsetScratch()

{

	if (false == IsValid(YonduNiagaraSystemAsset.Get()))

	{

		TObjectPtr<UNiagaraSystem> LoadedNiagaScratch = nullptr;

		YJJHelpers::GetAssetDynamic<UNiagaraSystem>(&LoadedNiagaScratch, FString(YonduNiagaraAssetPathScratch));

		YonduNiagaraSystemAsset = LoadedNiagaScratch.Get();

	}



	if (false == IsValid(YonduCascadeProjectileAsset.Get()))

	{

		TObjectPtr<UParticleSystem> LoadedCascadeScratch = nullptr;

		YJJHelpers::GetAssetDynamic<UParticleSystem>(&LoadedCascadeScratch, FString(YonduCascadeProjectilePathScratch));

		YonduCascadeProjectileAsset = LoadedCascadeScratch.Get();

	}

}



bool ACSkillYondu::SkillYondu_TargetAppearsLivingScratch(ACCommonCharacter* Candidate) const

{

	if (false == IsValid(Candidate))

	{

		return false;

	}



	UCCharacterStatComponent const* StatScratch = Candidate->GetYJJCharacterStatComponent();



	if (false == IsValid(StatScratch))

	{

		CLog::Log(FString::Printf(

			TEXT("[SkillYondu] StatComp 없음 — 피격자 생존 판정 불능, 해당 히트 스플라인 생략 — %s — %s"),

			*GetNameSafe(Candidate),

			*GetNameSafe(this)));

		return false;

	}



	return false == StatScratch->IsDead();

}



bool ACSkillYondu::SkillYondu_TryAddUniqueVictimScratch(

	TArray<TObjectPtr<ACCommonCharacter>>& InOutTargets, ACCommonCharacter* Candidate)

{

	if (false == IsValid(Candidate))

	{

		return false;

	}



	const int32 UpperScratch = InOutTargets.Num();

	for (int32 ixScratch = 0; ixScratch < UpperScratch; ++ixScratch)

	{

		if (Candidate == InOutTargets[ixScratch].Get())

		{

			return false;

		}

	}



	InOutTargets.Add(Candidate);

	return true;

}



void ACSkillYondu::SkillYondu_UpdateCapsuleHitCollisionScratch()

{

	UCapsuleComponent* CapsuleScratch = Capsule.Get();



	if (false == IsValid(CapsuleScratch))

	{

		return;

	}



	if (ProjectileState == EProjectileState_Yondu::Shooting)

	{

		CapsuleScratch->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		CapsuleScratch->SetGenerateOverlapEvents(true);

	}

	else

	{

		CapsuleScratch->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CapsuleScratch->SetGenerateOverlapEvents(false);

	}

}



void ACSkillYondu::SkillYondu_StartLifeTimerScratch()

{

	UWorld* WorldScratch = GetWorld();



	if (false == IsValid(WorldScratch))

	{

		return;

	}



	WorldScratch->GetTimerManager().ClearTimer(SkillYondu_LifeTimeTimerHandleScratch);



	const float PeriodScratch = FMath::Max(0.05f, LifeTimeSeconds);



	WorldScratch->GetTimerManager().SetTimer(

		SkillYondu_LifeTimeTimerHandleScratch,

		this,

		&ACSkillYondu::SkillYondu_TimeoutCallback,

		PeriodScratch,

		false);

}



void ACSkillYondu::SkillYondu_TimeoutCallback()

{

	UWorld const* WorldScratch = GetWorld();

	if ((false == HasAuthority()) || (false == IsValid(WorldScratch)))

	{

		return;

	}



	if (ProjectileState != EProjectileState_Yondu::Shooting)

	{

		return;

	}



	USceneComponent const* ArrowScratch = ArrotRoot.Get();

	if (false == IsValid(ArrowScratch))

	{

		CLog::Log(FString::Printf(TEXT("[SkillYondu] TimeOut: ArrotRoot 없음 — %s"), *GetNameSafe(this)));

		return;

	}



	WorldScratch->GetTimerManager().ClearTimer(SkillYondu_LifeTimeTimerHandleScratch);



	ProjectileState = EProjectileState_Yondu::Back;

	SkillYondu_MoveBackElapsedScratch = 0.0;

	SkillYondu_MoveBackStartWorldScratch = ArrowScratch->GetComponentTransform();



	SkillYondu_UpdateCapsuleHitCollisionScratch();



	// 수명 종료 귀환 구간에서는 연출만 정리하면 충분(블프는 상태만 전환).

}



void ACSkillYondu::SkillYondu_TickReadyScratch(float const DeltaSeconds)

{

	(void)DeltaSeconds;



	USceneComponent* ArrowScratch = ArrotRoot.Get();

	if (true == IsValid(ArrowScratch))

	{

		ArrowScratch->SetWorldTransform(MagicDefaultTransform);

	}

}



void ACSkillYondu::SkillYondu_TickShootingScratch(float const DeltaSeconds)

{

	USplineComponent* SplineScratch = Spline.Get();

	USceneComponent* ArrowScratch = ArrotRoot.Get();



	if ((false == IsValid(SplineScratch)) || (false == IsValid(ArrowScratch)))

	{

		return;

	}



	const float SplineLenScratch = SplineScratch->GetSplineLength();

	const float DtScratch = FMath::Max(0.f, DeltaSeconds);



	if ((SplineLenScratch <= KINDA_SMALL_NUMBER) || (DtScratch <= SMALL_NUMBER))

	{

		static_cast<void>(Shoot());

		return;

	}



	if (static_cast<double>(MovedLength) < static_cast<double>(SplineLenScratch))

	{

		const float MovedFloatScratch = static_cast<float>(MovedLength);



		FTransform const TransformAlongScratch = SplineScratch->GetTransformAtDistanceAlongSpline(

			MovedFloatScratch,

			ESplineCoordinateSpace::World,

			false);



		ArrowScratch->SetWorldLocationAndRotation(

			TransformAlongScratch.GetLocation(),

			TransformAlongScratch.Rotator());



		MovedLength += static_cast<double>(SkillSpeed) * static_cast<double>(DtScratch);

	}

	else

	{

		static_cast<void>(Shoot());

	}

}



void ACSkillYondu::SkillYondu_TickBackScratch(float const DeltaSeconds)

{

	USceneComponent* ArrowScratch = ArrotRoot.Get();



	if (false == IsValid(ArrowScratch))

	{

		CLog::Log(FString::Printf(TEXT("[SkillYondu] Back 틱: ArrotRoot 없음 — %s"), *GetNameSafe(this)));

		ProjectileState = EProjectileState_Yondu::Ready;

		SkillYondu_UpdateCapsuleHitCollisionScratch();

		return;

	}



	const float DurationScratch = FMath::Max(KINDA_SMALL_NUMBER, MoveBackSeconds);

	SkillYondu_MoveBackElapsedScratch += static_cast<double>(FMath::Max(0.f, DeltaSeconds));



	const float AlphaScratch = FMath::Clamp(

		static_cast<float>(SkillYondu_MoveBackElapsedScratch / static_cast<double>(DurationScratch)),

		0.f,

		1.f);



	const float EasedScratch = SkillYondu_EaseInCubicScratch(AlphaScratch);



	FTransform BlendedScratch;

	BlendedScratch.Blend(

		SkillYondu_MoveBackStartWorldScratch,

		MagicDefaultTransform,

		EasedScratch);



	ArrowScratch->SetWorldTransform(BlendedScratch);



	if (AlphaScratch >= (1.f - KINDA_SMALL_NUMBER))

	{

		ProjectileState = EProjectileState_Yondu::Ready;

		MovedLength = 0.0;



		if (true == IsValid(YonduNiagara.Get()))

		{

			YonduNiagara->Deactivate();

		}

		if (true == IsValid(YonduCascade.Get()))

		{

			YonduCascade->Deactivate();

		}



		SkillYondu_UpdateCapsuleHitCollisionScratch();

	}

}



void ACSkillYondu::SkillYondu_OnCapsuleBeginOverlap(

	UPrimitiveComponent* OverlappedComponent,

	AActor* OtherActor,

	UPrimitiveComponent* OtherComp,

	int32 OtherBodyIndex,

	bool bFromSweep,

	const FHitResult& SweepResult)

{

	(void)OverlappedComponent;

	(void)OtherComp;

	(void)OtherBodyIndex;

	(void)bFromSweep;

	(void)SweepResult;



	if (false == HasAuthority())

	{

		return;

	}



	if ((ProjectileState != EProjectileState_Yondu::Shooting) || (false == IsValid(OtherActor)))

	{

		return;

	}



	const int32 IgnoreUpperScratch = ActorsToIgnore.Num();

	for (int32 igIxScratch = 0; igIxScratch < IgnoreUpperScratch; ++igIxScratch)

	{

		if (OtherActor == ActorsToIgnore[igIxScratch].Get())

		{

			return;

		}

	}



	ACCommonCharacter* VictimScratch = Cast<ACCommonCharacter>(OtherActor);

	if (false == IsValid(VictimScratch))

	{

		return;

	}



	ACCommonCharacter* CasterScratch = Character.Get();

	if (false == IsValid(CasterScratch))

	{

		CLog::Log(FString::Printf(

			TEXT("[SkillYondu] Overlap: Character(시전자) 없음 — 피격 생략 — %s"),

			*GetNameSafe(this)));

		return;

	}



	if (true == UCYJJBlueprintLibrary::AreCharactersSameGroup(CasterScratch, VictimScratch))

	{

		return;

	}



	if (false == SkillYondu_TargetAppearsLivingScratch(VictimScratch))

	{

		return;

	}



	const ACMagic* MagicScratch = Magic.Get();



	if (false == IsValid(MagicScratch))

	{

		CLog::Log(FString::Printf(TEXT("[SkillYondu] Overlap: Magic 없음 — 피격 생략 — %s"), *GetNameSafe(this)));

		return;

	}



	if (false == MagicScratch->HitCommonDatas.IsValidIndex(0))

	{

		CLog::Log(FString::Printf(

			TEXT("[SkillYondu] HitCommonDatas[0] 없음 — 피격 생략 — Magic=%s — %s"),

			*GetNameSafe(MagicScratch),

			*GetNameSafe(this)));

		return;

	}



	UCapsuleComponent const* CapsuleScratch = Capsule.Get();

	FVector HitPointScratch = true == IsValid(CapsuleScratch) ? CapsuleScratch->GetComponentLocation()

															  : VictimScratch->GetActorLocation();



	const FHitData& RowScratch = MagicScratch->HitCommonDatas[0];



	VictimScratch->SetDamagedInfo(CasterScratch, this, RowScratch, HitPointScratch);



	RowScratch.SendDamage(

		TWeakObjectPtr<ACCommonCharacter>(CasterScratch),

		TWeakObjectPtr<AActor>(this),

		TWeakObjectPtr<ACCommonCharacter>(VictimScratch));

}



void ACSkillYondu::Tick(float DeltaSeconds)

{

	Super::Tick(DeltaSeconds);



	if (false == HasAuthority())

	{

		return;

	}



	UWorld const* WorldScratch = GetWorld();



	if (false == IsValid(WorldScratch))

	{

		return;

	}



	switch (ProjectileState)

	{

	case EProjectileState_Yondu::None:

	default:

		break;



	case EProjectileState_Yondu::Ready:

		SkillYondu_TickReadyScratch(DeltaSeconds);

		break;



	case EProjectileState_Yondu::Shooting:

		SkillYondu_TickShootingScratch(DeltaSeconds);

		break;



	case EProjectileState_Yondu::Back:

		SkillYondu_TickBackScratch(DeltaSeconds);

		break;

	}

}



void ACSkillYondu::SkillYondu_MulticastStartFlightScratch_Implementation()

{

	if (false == SkillYondu_ShouldRunCosmeticsScratch())

	{

		return;

	}



	if ((true == IsValid(YonduNiagara.Get())) && (false == YonduNiagara->IsActive()))

	{

		YonduNiagara->Activate(true);

	}



	if ((true == IsValid(YonduCascade.Get())) && (false == YonduCascade->IsActive()))

	{

		YonduCascade->Activate(true);

	}

}



void ACSkillYondu::Shoot_Implementation()

{

	UWorld* WorldScratch = GetWorld();



	if (false == HasAuthority())

	{

		return;

	}



	if (false == IsValid(WorldScratch))

	{

		CLog::Log(FString::Printf(TEXT("[SkillYondu] Shoot: World 없음 — %s"), *GetNameSafe(this)));

		return;

	}



	USplineComponent* SplineScratch = Spline.Get();

	UCapsuleComponent* CapsuleScratch = Capsule.Get();

	USceneComponent* ArrowScratch = ArrotRoot.Get();



	if ((false == IsValid(SplineScratch)) || (false == IsValid(CapsuleScratch)) || (false == IsValid(ArrowScratch)))

	{

		CLog::Log(FString::Printf(

			TEXT("[SkillYondu] 필수 컴포넌트 없음(`Spline`|`Capsule`|`ArrotRoot`) — %s"),

			*GetNameSafe(this)));

		return;

	}



	WorldScratch->GetTimerManager().ClearTimer(SkillYondu_LifeTimeTimerHandleScratch);



	ActorsToIgnore.Reset();



	Targets.Reset();



	CurIndex = 0;

	MovedLength = 0.0;

	ProjectileState = EProjectileState_Yondu::Shooting;



	SplineScratch->ClearSplinePoints(true);



	SkillYondu_RebuildActorsToIgnoreScratch();



	const FVector SphereCenterScratch = ArrowScratch->GetComponentLocation();



	ACCommonCharacter* CasterScratch = Character.Get();



	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectQueriesScratch;



	ObjectQueriesScratch.Reserve(1);



	ObjectQueriesScratch.Add(EObjectTypeQuery::ObjectTypeQuery3);



	TArray<AActor*> RawIgnoreActorsScratch;



	const int32 IgnoreUpperScratch = ActorsToIgnore.Num();

	RawIgnoreActorsScratch.Reserve(IgnoreUpperScratch + 6);



	for (int32 igIxScratch = 0; igIxScratch < IgnoreUpperScratch; ++igIxScratch)

	{

		AActor* IgnoreActorScratch = ActorsToIgnore[igIxScratch].Get();

		if (true == IsValid(IgnoreActorScratch))

		{

			RawIgnoreActorsScratch.Add(IgnoreActorScratch);

		}

	}



	RawIgnoreActorsScratch.AddUnique(this);



	if (AActor* OwnerScratch = GetOwner())

	{

		RawIgnoreActorsScratch.AddUnique(OwnerScratch);

	}



	TArray<FHitResult> HitsScratch;



	const float RadiusScratch = FMath::Max(1.f, SkillYonduTraceRadiusUU);



	static_cast<void>(UKismetSystemLibrary::SphereTraceMultiForObjects(

		WorldScratch,

		SphereCenterScratch,

		SphereCenterScratch,

		RadiusScratch,

		ObjectQueriesScratch,

		false,

		RawIgnoreActorsScratch,

		EDrawDebugTrace::None,

		HitsScratch,

		true));



	const int32 HitsUpperScratch = HitsScratch.Num();



	for (int32 hitIxScratch = 0; hitIxScratch < HitsUpperScratch; ++hitIxScratch)

	{

		FHitResult const& HitScratch = HitsScratch[hitIxScratch];



		if (false == HitScratch.bBlockingHit)

		{

			continue;

		}



		ACCommonCharacter* VictimScratch = Cast<ACCommonCharacter>(HitScratch.GetActor());

		if (false == IsValid(VictimScratch))

		{

			continue;

		}



		if (false == SkillYondu_TargetAppearsLivingScratch(VictimScratch))

		{

			continue;

		}



		bool const SameGroupScratch =

			IsValid(CasterScratch) &&

			true == UCYJJBlueprintLibrary::AreCharactersSameGroup(CasterScratch, VictimScratch);

		if (true == SameGroupScratch)

		{

			continue;

		}



		const bool AddedScratch = SkillYondu_TryAddUniqueVictimScratch(Targets, VictimScratch);

		if (false == AddedScratch)

		{

			continue;

		}



		FVector const VictimLocScratch = VictimScratch->GetActorLocation();

		FVector ToVictimScratch = VictimLocScratch - SphereCenterScratch;

		ToVictimScratch.Normalize();



		FVector AlongScratch = SkillYonduSplineAlongVictimUU * ToVictimScratch;

		FVector RandZScratch =

			FVector(0.f, 0.f, FMath::FRandRange(0.f, SkillYonduRandomZMaxUUScratch));

		FVector const SecondScratch = VictimLocScratch + AlongScratch + RandZScratch;



		SplineScratch->AddSplinePoint(VictimLocScratch, ESplineCoordinateSpace::World, false);

		SplineScratch->AddSplinePoint(SecondScratch, ESplineCoordinateSpace::World, true);

	}



	CurIndex = 0;



	if (SplineScratch->GetSplineLength() <= KINDA_SMALL_NUMBER)

	{

		ProjectileState = EProjectileState_Yondu::Ready;

		MovedLength = 0.0;



		if (true == IsValid(YonduNiagara.Get()))

		{

			YonduNiagara->Deactivate();

		}

		if (true == IsValid(YonduCascade.Get()))

		{

			YonduCascade->Deactivate();

		}



		SkillYondu_UpdateCapsuleHitCollisionScratch();



		CLog::Log(FString::Printf(

			TEXT("[SkillYondu] Shoot: 스플라인 길이 0(타깃 없음) — Ready 로 복귀 — %s"),

			*GetNameSafe(this)));

		return;

	}



	SkillYondu_UpdateCapsuleHitCollisionScratch();



	SkillYondu_StartLifeTimerScratch();



	SkillYondu_MulticastStartFlightScratch();

}


