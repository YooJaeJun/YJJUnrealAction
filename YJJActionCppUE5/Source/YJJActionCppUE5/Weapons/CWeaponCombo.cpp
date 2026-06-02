#include "Weapons/CWeaponCombo.h"

#include "Commons/CYJJBlueprintLibrary.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CTargetingComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Components/CWeaponComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Controller.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Utilities/CLog.h"
#include "Weapons/CAttachment.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "TimerManager.h"

namespace
{
	constexpr float ComboTickTargetingInterpSpeed = 5.0f;
	constexpr float ComboTickTargetingRotatorTolerance = 1.0f;
	constexpr float ComboTickTargetingMouseTolerance = 1.0f;
	constexpr float ComboBeginFlyingNotifyAddGravityFactor = 1.2f;
	constexpr float ComboBeginAirLaunchUp = 1000.0f;
	constexpr float ComboFallDownGravityDelaySeconds = 0.3f;
	// ?덇굅??`Weapon_Combo:FallDownGravity` ??Moving `SetGravity(InGravity)` ? 湲곕낯媛?= `CharacterMovement::GravityScale`).
	constexpr float ComboFallDownGravityBpGravityScale = 20.f;
	// ?덇굅??`Weapon_Guard` Tick `ClearHitted` 釉뚮옖移?`Delay`(0.8s).
	constexpr float GuardClearHittedDelaySeconds = 0.8f;

	AController* ComboResolveControllerForWeaponContext(ACWeapon* InWeapon)
	{
		if (false == IsValid(InWeapon))
		{
			return nullptr;
		}

		if (IsValid(InWeapon->Controller))
		{
			return InWeapon->Controller;
		}

		if (false == IsValid(InWeapon->Character))
		{
			return nullptr;
		}

		ACCommonCharacter* AsCommon = Cast<ACCommonCharacter>(InWeapon->Character);
		if (IsValid(AsCommon))
		{
			const TWeakObjectPtr<AController> Cur = AsCommon->GetMyCurController();
			if (Cur.IsValid())
			{
				return Cur.Get();
			}

			return AsCommon->GetController();
		}

		return InWeapon->Character->GetController();
	}
} // namespace
ACWeaponCombo::ACWeaponCombo()
{
}

ACWeaponGuard::ACWeaponGuard()
{
}

namespace WeaponGuard_Local
{
	static UShapeComponent* FindShape(ACWeaponGuard* Self, const int32 IndexRaw)
	{
		if (false == IsValid(Self))
		{
			return nullptr;
		}

		if (Self->Collisions.Num() <= 0)
		{
			return nullptr;
		}

		const int32 Clamped = Self->Collisions.IsValidIndex(IndexRaw)
			? IndexRaw
			: FMath::Clamp(IndexRaw, 0, Self->Collisions.Num() - 1);
		UShapeComponent* Shape = Self->Collisions[Clamped].Get();
		if (false == IsValid(Shape))
		{
			ensureMsgf(false, TEXT("[WeaponGuardSkillContext] Collisions[%d] Shape 臾댄슚 ??%s"),
				Clamped,
				*GetNameSafe(Self));
		}
		return Shape;
	}

	static UPrimitiveComponent* FindAnyWeaponOverlapPrim(ACWeapon* WeaponCtx)
	{
		if (false == IsValid(WeaponCtx))
		{
			return nullptr;
		}

		TArray<UShapeComponent*> Scratch;
		WeaponCtx->GetComponents<UShapeComponent>(Scratch, true);
		if (Scratch.Num() <= 0)
		{
			return nullptr;
		}

		return Scratch[0];
	}
}

void ACWeaponGuard::BeginPlay()
{
	Super::BeginPlay();

	Collisions.Reset();
	TArray<UShapeComponent*> ScratchShapes;
	GetComponents<UShapeComponent>(ScratchShapes, true);
	Collisions.Reserve(ScratchShapes.Num());
	for (UShapeComponent* ShapeCandidate : ScratchShapes)
	{
		if (false == IsValid(ShapeCandidate))
			continue;

		Collisions.Add(ShapeCandidate);
	}

	OffGuardCollsion();
	OffSubWeaponCollision();
	Guard_BindOverlapsRecursive();
}

void ACWeaponGuard::Guard_BindOverlapsRecursive()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : Collisions)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* Prim = ShapePtr;
		Prim->OnComponentBeginOverlap.AddDynamic(this, &ACWeaponGuard::Guard_OnShapeBeginOverlap_Dynamic);
	}
}

void ACWeaponGuard::Guard_UnbindOverlapsRecursive()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : Collisions)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* Prim = ShapePtr;
		Prim->OnComponentBeginOverlap.RemoveDynamic(this,
			&ACWeaponGuard::Guard_OnShapeBeginOverlap_Dynamic);
	}
}

void ACWeaponGuard::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Guard_UnbindOverlapsRecursive();
	Collisions.Reset();

	UWorld* WorldForTimers = GetWorld();
	if (IsValid(WorldForTimers))
	{
		FTimerManager& TM = WorldForTimers->GetTimerManager();
		TM.ClearTimer(GuardParriableResetTimerHandle);
		TM.ClearTimer(GuardParriedReflectTimerHandle);
		TM.ClearTimer(GuardClearHittedTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ACWeaponGuard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		return;
	}

	FTimerManager& TM = World->GetTimerManager();

	if (Parriable)
	{
		if (false == TM.IsTimerActive(GuardParriableResetTimerHandle))
		{
			TM.SetTimer(GuardParriableResetTimerHandle,
				FTimerDelegate::CreateUObject(this, &ACWeaponGuard::Guard_Timer_ClearParriableElapsed),
				static_cast<float>(ParriableDelay),
				false);
		}
	}

	if (Parried)
	{
		Parried = false;
		if (false == TM.IsTimerActive(GuardParriedReflectTimerHandle))
		{
			TM.SetTimer(GuardParriedReflectTimerHandle,
				FTimerDelegate::CreateUObject(this, &ACWeaponGuard::Guard_Timer_ParriedReflectElapsed),
				static_cast<float>(ParriedDelay),
				false);
		}
	}

	if (ClearHitted)
	{
		if (false == TM.IsTimerActive(GuardClearHittedTimerHandle))
		{
			TM.SetTimer(GuardClearHittedTimerHandle,
				FTimerDelegate::CreateUObject(this, &ACWeaponGuard::Guard_Timer_ClearHittedListElapsed),
				GuardClearHittedDelaySeconds,
				false);
		}
	}
}

void ACWeaponGuard::Guard_Timer_ClearParriableElapsed()
{
	Parriable = false;
}

void ACWeaponGuard::Guard_Timer_ParriedReflectElapsed()
{
	ACharacter* TargetRaw = Target.Get();
	ACCommonCharacter* TargetChar = Cast<ACCommonCharacter>(TargetRaw);

	if (!ensureMsgf(IsValid(TargetChar), TEXT("[WeaponGuardSkillContext] ?⑤쭅 諛섏궗 ?寃잛? ACCommonCharacter ?ъ빞 ????%s"),
			*GetNameSafe(TargetRaw)))
	{
		Target = nullptr;
		return;
	}

	if (!ensureMsgf(IsValid(Character), TEXT("[WeaponGuardSkillContext] ?⑤쭅 諛섏궗: ?뚯쑀 Character ?놁쓬 ??%s"),
			*GetNameSafe(this)))
	{
		Target = nullptr;
		return;
	}

	if (!ensureMsgf(HitCommonDatas.IsValidIndex(0),
			TEXT("[WeaponGuardSkillContext] ?⑤쭅 諛섏궗: HitCommonDatas[0] ?꾩슂 ??%s"),
			*GetNameSafe(this)))
	{
		Target = nullptr;
		return;
	}

	HitCommonDatas[0].SendDamage(
		TWeakObjectPtr<ACCommonCharacter>(Character),
		TWeakObjectPtr<AActor>(this),
		TWeakObjectPtr<ACCommonCharacter>(TargetChar));

	Target = nullptr;
}

void ACWeaponGuard::Guard_Timer_ClearHittedListElapsed()
{
	ClearHitted = false;
	Hitted.Reset();
}

void ACWeaponGuard::OnGuardCollision()
{
	UShapeComponent* Shape = WeaponGuard_Local::FindShape(this, GuardCollisionIndex);
	if (!ensureMsgf(IsValid(Shape),
			TEXT("[WeaponGuardSkillContext] OnGuardCollision: GuardCollisionIndex Shape ?놁쓬(idx=%d) ??%s"),
			GuardCollisionIndex,
			*GetNameSafe(this)))
	{
		return;
	}

	Shape->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ACWeaponGuard::OffGuardCollsion()
{
	UShapeComponent* Shape = WeaponGuard_Local::FindShape(this, GuardCollisionIndex);
	if (!ensureMsgf(IsValid(Shape),
			TEXT("[WeaponGuardSkillContext] OffGuardCollsion: Shape ?놁쓬(idx=%d) ??%s"),
			GuardCollisionIndex,
			*GetNameSafe(this)))
	{
		return;
	}

	Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACWeaponGuard::OnSubWeaponCollision()
{
	UShapeComponent* Shape = WeaponGuard_Local::FindShape(this, ActionCollisionIndex);
	if (!ensureMsgf(IsValid(Shape),
			TEXT("[WeaponGuardSkillContext] OnSubWeaponCollision: Shape ?놁쓬(idx=%d) ??%s"),
			ActionCollisionIndex,
			*GetNameSafe(this)))
	{
		return;
	}

	Shape->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void ACWeaponGuard::OffSubWeaponCollision()
{
	UShapeComponent* Shape = WeaponGuard_Local::FindShape(this, ActionCollisionIndex);
	if (!ensureMsgf(IsValid(Shape),
			TEXT("[WeaponGuardSkillContext] OffSubWeaponCollision: Shape ?놁쓬(idx=%d) ??%s"),
			ActionCollisionIndex,
			*GetNameSafe(this)))
	{
		return;
	}

	Shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACWeaponGuard::Guard_OnShapeBeginOverlap_Dynamic(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepResult;

	UShapeComponent* OverlapShape = Cast<UShapeComponent>(OverlappedComponent);
	if (!ensureMsgf(IsValid(OverlapShape),
			TEXT("[WeaponGuardSkillContext] 媛???ㅻ쾭??諛붿씤 ??곸? Shape ?ъ빞 ????%s"),
			*GetNameSafe(OverlappedComponent)))
	{
		return;
	}

	if (false == IsValid(Character))
	{
		return;
	}

	ACCommonCharacter* IncomingAttackerChar = nullptr;
	UPrimitiveComponent* TraceOtherPrim = OtherComp;

	ACWeapon* OverlapWeaponCtx = Cast<ACWeapon>(OtherActor);
	if (IsValid(OverlapWeaponCtx))
	{
		IncomingAttackerChar = OverlapWeaponCtx->Character;
		if (false == IsValid(IncomingAttackerChar))
		{
			IncomingAttackerChar = Cast<ACCommonCharacter>(OverlapWeaponCtx->GetOwner());
		}

		UPrimitiveComponent* WeaponShapeHint = WeaponGuard_Local::FindAnyWeaponOverlapPrim(OverlapWeaponCtx);
		if (IsValid(WeaponShapeHint))
		{
			TraceOtherPrim = WeaponShapeHint;
		}
	}
	else
	{
		IncomingAttackerChar = Cast<ACCommonCharacter>(OtherActor);
	}

	if (false == IsValid(IncomingAttackerChar))
	{
		return;
	}

	if (UCYJJBlueprintLibrary::AreCharactersSameGroup(Character, IncomingAttackerChar))
	{
		return;
	}

	FVector ImpactWorld =
		IsValid(TraceOtherPrim) ? TraceOtherPrim->K2_GetComponentLocation() : IncomingAttackerChar->GetActorLocation();

	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Character);

	bool TraceOk = UCYJJBlueprintLibrary::TryGetHitPointBetweenPrimitives(
		OverlapShape,
		TraceOtherPrim,
		GuardHitSphereRadius,
		IgnoreActors,
		ImpactWorld);

	if (false == TraceOk)
	{
		if (true == IsValid(TraceOtherPrim))
		{
			ImpactWorld = TraceOtherPrim->K2_GetComponentLocation();
		}
		else
		{
			ImpactWorld = IncomingAttackerChar->GetActorLocation();
		}
	}

	OnBeginOverlap(IncomingAttackerChar, ImpactWorld);
}

void ACWeaponGuard::OnBeginOverlap_Implementation(ACCommonCharacter* InCharacter,
	const FVector& InHitPoint)
{
	if (!ensureMsgf(IsValid(InCharacter), TEXT("[WeaponGuardSkillContext] OnBeginOverlap: InCharacter ?꾩슂")))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!ensureMsgf(IsValid(World), TEXT("[WeaponGuardSkillContext] OnBeginOverlap: World ?꾩슂 ??%s"), *GetNameSafe(this)))
	{
		return;
	}

	FTimerManager* TimerMgr = &World->GetTimerManager();

	if (Guarding)
	{
		TArray<ACCommonCharacter*> Scratch;
		Scratch.Reserve(Hitted.Num() + 2);
		for (TObjectPtr<ACCommonCharacter>& EntryWeak : Hitted)
		{
			ACCommonCharacter* EntryRaw = EntryWeak.Get();
			if (IsValid(EntryRaw))
			{
				Scratch.Add(EntryRaw);
			}
		}

		const bool bInserted = UCYJJBlueprintLibrary::TryAddUniqueCommonCharacter(Scratch, InCharacter);

		Hitted.Reset();
		Hitted.Reserve(Scratch.Num());
		for (ACCommonCharacter* Added : Scratch)
		{
			Hitted.Add(Added);
		}

		if (bInserted)
		{
			const FVector SoundLocation =
				IsValid(Character) ? Character->GetActorLocation() : GetActorLocation();

			if (IsValid(GuardBlockSoundAsset))
			{
				UGameplayStatics::PlaySoundAtLocation(World,
					GuardBlockSoundAsset,
					SoundLocation,
					FRotator::ZeroRotator,
					1.0f,
					1.0f,
					0.0f,
					GuardBlockAttenuation);
			}

			ClearHitted = true;
		}
	}

	if (Parriable)
	{
		if (!ensureMsgf(DoActionDatas.IsValidIndex(ParryingAnimIndex),
				TEXT("[WeaponGuardSkillContext] ?⑤쭅 ?ъ깮 ?ㅽ뙣: DoActionDatas ???몃뜳??%d ?놁쓬(????%d) ??%s"),
				ParryingAnimIndex,
				DoActionDatas.Num(),
				*GetNameSafe(this)))
		{
			return;
		}

		TimerMgr->ClearTimer(GuardParriableResetTimerHandle);

		Parriable = false;
		Guarding = false;
		Parrying = true;

		PlayAction(DoActionDatas, ParryingAnimIndex, InHitPoint);

		Parried = true;
		Target = InCharacter;
	}
}

void ACWeaponGuard::Hold_SubWeapon_Implementation()
{
	Parriable = true;

	if (Parrying)
	{
		// ?덇굅??洹몃옒?? Parrying 遺꾧린??then ? 鍮?寃곌낵 ??Guarding/PlayAction ??吏꾪뻾?섏? ?딅뒗??Idle 遺꾧린 吏꾩엯 遺덇?? ?숇벑).
		return;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (!ensureMsgf(IsValid(StateComp), TEXT("[WeaponGuardSkillContext] Hold_SubWeapon: StateComponent ?꾩슂 ??%s"),
			*GetNameSafe(this)))
	{
		return;
	}

	if (false == StateComp->IsIdle() && false == StateComp->IsHitted())
	{
		return;
	}

	if (!ensureMsgf(DoActionDatas.Num() > 0, TEXT("[WeaponGuardSkillContext] Hold_SubWeapon: DoActionDatas ?꾩슂 ??%s"),
			*GetNameSafe(this)))
	{
		return;
	}

	Guarding = true;

	const int32 ClampedGuardIndex = FMath::Clamp(GuardAnimIndex, 0, DoActionDatas.Num() - 1);
	const FVector HitPointZero = FVector::ZeroVector;
	PlayAction(DoActionDatas, ClampedGuardIndex, HitPointZero);
}

void ACWeaponGuard::Released_SubWeapon_Implementation()
{
	Guarding = false;

	ACharacter* CharActor = IsValid(Character) ? Cast<ACharacter>(Character) : nullptr;
	if (!ensureMsgf(IsValid(CharActor), TEXT("[WeaponGuardSkillContext] Released_SubWeapon: ACharacter ?꾩슂 ??%s"),
			*GetNameSafe(this)))
	{
		return;
	}

	if (!ensureMsgf(IsValid(GuardHoldStopMontage),
			TEXT("[WeaponGuardSkillContext] Released_SubWeapon: GuardHoldStopMontage ?좊떦 ?꾩슂 ??%s"),
			*GetNameSafe(this)))
	{
		return;
	}

	CharActor->StopAnimMontage(GuardHoldStopMontage);

	if (!ensureMsgf(IsValid(Moving), TEXT("[WeaponGuardSkillContext] Released_SubWeapon: Moving ?꾩슂 ??%s"),
			*GetNameSafe(this)))
	{
		return;
	}

	Moving->Move();
}

void ACWeaponGuard::DoSubWeaponAction_Implementation()
{
	// ?덇굅??`Weapon_Guard:DoSubWeaponAction` ???몃뱶 誘몄뿰寃곕줈 蹂몃Ц ?놁쓬(`Weapon` 湲곕낯??`CancelDashes` ???몄텧 ????.
}

void ACWeaponGuard::Begin_DoSubWeaponAction_Implementation()
{
	Super::Begin_DoSubWeaponAction_Implementation();

	UWorld* World = GetWorld();
	if (!ensureMsgf(IsValid(World), TEXT("[WeaponGuardSkillContext] Begin_DoSubWeaponAction: World ?꾩슂 ??%s"),
			*GetNameSafe(this)))
	{
		return;
	}

	if (!ensureMsgf(HitCommonDatas.Num() > 0,
			TEXT("[WeaponGuardSkillContext] Begin_DoSubWeaponAction: HitCommonDatas ?꾩슂 ??%s"),
			*GetNameSafe(this)))
	{
		return;
	}

	const float HitStopValue = HitCommonDatas[0].HitStop;

	TArray<AActor*> Candidates;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Candidates);

	for (AActor* Candidate : Candidates)
	{
		if (false == IsValid(Candidate))
		{
			continue;
		}

		bool bShouldDilate = false;
		const APawn* AsPawn = Cast<APawn>(Candidate);
		if (IsValid(AsPawn))
		{
			bShouldDilate = true;
		}
		else
		{
			UMeshComponent* MeshComp = Candidate->FindComponentByClass<UMeshComponent>();
			if (IsValid(MeshComp) && MeshComp->Mobility == EComponentMobility::Movable)
			{
				bShouldDilate = true;
			}
		}

		if (false == bShouldDilate)
		{
			continue;
		}

		DilationActors.Add(Candidate);
		Candidate->CustomTimeDilation = HitStopValue;
	}
}

void ACWeaponGuard::End_DoSubWeaponAction_Implementation()
{
	Parrying = false;

	for (TObjectPtr<AActor>& EntryWeak : DilationActors)
	{
		AActor* ActorPtr = EntryWeak.Get();
		if (IsValid(ActorPtr))
		{
			ActorPtr->CustomTimeDilation = 1.0f;
		}
	}

	DilationActors.Reset();

	Super::End_DoSubWeaponAction_Implementation();
}

void ACWeaponCombo::BeginPlay()
{
	Super::BeginPlay();

	ComboCollisionShapes.Reset();
	GetComponents<UShapeComponent>(ComboCollisionShapes, true);
	OffCollisions();

	for (TObjectPtr<UShapeComponent>& ShapeWeak : ComboCollisionShapes)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* PrimPtr = ShapePtr;

		PrimPtr->OnComponentBeginOverlap.AddDynamic(
			this,
			&ACWeaponCombo::ComboOnShapeBeginOverlap_Impl);

		PrimPtr->OnComponentHit.AddDynamic(this, &ACWeaponCombo::ComboOnShapeHit_Impl);
	}
}

void ACWeaponCombo::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : ComboCollisionShapes)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		UPrimitiveComponent* PrimPtr = ShapePtr;
		PrimPtr->OnComponentBeginOverlap.RemoveDynamic(
			this,
			&ACWeaponCombo::ComboOnShapeBeginOverlap_Impl);
		PrimPtr->OnComponentHit.RemoveDynamic(this, &ACWeaponCombo::ComboOnShapeHit_Impl);
	}

	ComboCollisionShapes.Reset();

	UWorld* WorldForTimer = GetWorld();
	if (IsValid(WorldForTimer))
	{
		WorldForTimer->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void ACWeaponCombo::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	Combo_Tick_TargetingDelegation();
}

void ACWeaponCombo::OnCollisions()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : ComboCollisionShapes)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		ShapePtr->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void ACWeaponCombo::OnBoxCollisions()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : ComboCollisionShapes)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		if (false == ShapePtr->IsA<UBoxComponent>())
		{
			continue;
		}

		ShapePtr->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void ACWeaponCombo::OffCollisions()
{
	for (TObjectPtr<UShapeComponent>& ShapeWeak : ComboCollisionShapes)
	{
		UShapeComponent* ShapePtr = ShapeWeak.Get();
		if (false == IsValid(ShapePtr))
		{
			continue;
		}

		ShapePtr->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// ?덇굅??Weapon_Combo::OffCollisions: Shape ?꾧퀬 ??Moving 怨좎젙 移대찓?쇰㈃ Hitted 瑜??꾨낫濡??꾨갑 1紐?怨⑤씪 RotationTarget/Targeting/Mouse 梨꾩슫 ????Hitted ?대━??
	const bool bRunFixedCameraTargeting =
		IsValid(Character) && IsValid(Moving) && Moving->GetFixedCamera();

	if (bRunFixedCameraTargeting)
	{
		TArray<ACharacter*> HitCandidates;
		HitCandidates.Reserve(ComboHitted.Num());

		for (TObjectPtr<ACCommonCharacter>& HitWeak : ComboHitted)
		{
			ACCommonCharacter* HitCharacter = HitWeak.Get();
			if (IsValid(HitCharacter))
			{
				HitCandidates.Add(HitCharacter);
			}
		}

		ACharacter* FrontCandidate = UCYJJBlueprintLibrary::FindNearlyFrontCharacter(
			Character,
			HitCandidates,
			0.7);

		if (IsValid(FrontCandidate))
		{
			const FVector CharacterLocation = Character->GetActorLocation();
			const FVector FrontLocation = FrontCandidate->GetActorLocation();
			ComboRotationTarget = UKismetMathLibrary::FindLookAtRotation(CharacterLocation, FrontLocation);
			ComboTargetingFlag = true;
			ComboMousePosition = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
		}
	}

	ComboHitted.Reset();
}

void ACWeaponCombo::EnableCombo()
{
	ComboEnable = true;
}

void ACWeaponCombo::DisableCombo()
{
	ComboEnable = false;
}

void ACWeaponCombo::Host_OnCollisions()
{
	OnCollisions();
}

void ACWeaponCombo::Host_OffCollisions()
{
	OffCollisions();
}

void ACWeaponCombo::Host_OnBoxCollisions()
{
	OnBoxCollisions();
}

void ACWeaponCombo::Host_EnableCombo()
{
	EnableCombo();
}

void ACWeaponCombo::Host_DisableCombo()
{
	DisableCombo();
}

void ACWeaponCombo::Combo_Tick_FixedCameraLookTarget_MouseEnd()
{
	// Tick 寃쎈줈: 留??꾨젅???몄텧濡??먯＜ ?ㅽ뙣 ??濡쒓렇 ?놁쓬(?꾨줈?앺듃 洹쒖튃).
	if (false == ComboTargetingFlag || false == IsValid(Character))
	{
		return;
	}

	AController* ControllerResolved = ComboResolveControllerForWeaponContext(this);

	if (IsValid(ControllerResolved))
	{
		const float DeltaSeconds = static_cast<float>(UGameplayStatics::GetWorldDeltaSeconds(GetWorld()));

		const FRotator CurrentRotation = ControllerResolved->GetControlRotation();
		const FRotator InterpRotation = UKismetMathLibrary::RInterpTo(
			CurrentRotation,
			ComboRotationTarget,
			DeltaSeconds,
			ComboTickTargetingInterpSpeed);

		ControllerResolved->SetControlRotation(InterpRotation);
	}

	const FVector2D MouseNow = UWidgetLayoutLibrary::GetMousePositionOnPlatform();
	const bool bMouseMoved = false
		== UKismetMathLibrary::EqualEqual_Vector2DVector2D(
			MouseNow,
			ComboMousePosition,
			ComboTickTargetingMouseTolerance);

	FRotator RotationAfter = FRotator::ZeroRotator;
	if (IsValid(ControllerResolved))
	{
		RotationAfter = ControllerResolved->GetControlRotation();
	}
	else if (IsValid(Character))
	{
		RotationAfter = Character->GetControlRotation();
	}

	const bool bReachedLookTarget = UKismetMathLibrary::EqualEqual_RotatorRotator(
		RotationAfter,
		ComboRotationTarget,
		ComboTickTargetingRotatorTolerance);

	if (bMouseMoved || bReachedLookTarget)
	{
		ComboTargetingFlag = false;
		if (IsValid(Moving))
		{
			Moving->UnFixCamera();
		}
	}
}

void ACWeaponCombo::Combo_Tick_TargetingDelegation()
{
	if (false == InEquip || false == IsValid(Character))
	{
		return;
	}

	// 怨좎젙 移대찓???寃????뚯쟾 蹂닿컙 ??`UCTargetingComponent` ?좉툑怨?蹂꾨룄 蹂??`ComboTargetingFlag`)濡??숈옉.
	if (true == ComboTargetingFlag)
	{
		Combo_Tick_FixedCameraLookTarget_MouseEnd();
		return;
	}

	UCTargetingComponent* TargetingCompRaw = Character->GetTargetingComp().Get();
	if (false == IsValid(TargetingCompRaw))
	{
		return;
	}

	// 而댄룷?뚰듃 ?깆씠 耳쒖졇 ?덉쑝硫??대떦 ???덉뿉???대? Tick_Targeting ???뚯븘媛꾨떎.
	if (TargetingCompRaw->IsComponentTickEnabled())
	{
		return;
	}

	TargetingCompRaw->Tick_Targeting();
}

bool ACWeaponCombo::Combo_ResolveHitDatasForMelee(
	ACWeapon* Weapon,
	const CEAttackType AttackTypeBranch,
	const int32 IndexCommon,
	const int32 IndexAir,
	const int32 IndexFlying,
	const int32 IndexDown,
	const int32 IndexSwaying,
	const int32 IndexFallDown,
	const int32 IndexSkill,
	TArray<FHitData>*& OutHitDatasPtr,
	int32& OutClampedHitIndex)
{
	if (false == IsValid(Weapon))
	{
		return false;
	}

	TArray<FHitData>* HitRowArray = nullptr;
	int32 HitIndexDesired = 0;

	switch (AttackTypeBranch)
	{
	case CEAttackType::Common:
		HitRowArray = &Weapon->HitCommonDatas;
		HitIndexDesired = IndexCommon;
		break;
	case CEAttackType::Air:
		HitRowArray = &Weapon->HitAirComboDatas;
		HitIndexDesired = IndexAir;
		break;
	case CEAttackType::Flying:
		HitRowArray = &Weapon->HitFlyingAttackDatas;
		HitIndexDesired = IndexFlying;
		break;
	case CEAttackType::Down:
		HitRowArray = &Weapon->HitDownAttackDatas;
		HitIndexDesired = IndexDown;
		break;
	case CEAttackType::DashAttack:
		HitRowArray = &Weapon->HitSwayingAttackDatas;
		HitIndexDesired = IndexSwaying;
		break;
	case CEAttackType::FallDown:
		HitRowArray = &Weapon->HitFallDownAttackDatas;
		HitIndexDesired = IndexFallDown;
		break;
	case CEAttackType::Skill:
		HitRowArray = &Weapon->HitSkillDatas;
		HitIndexDesired = IndexSkill;
		break;
	default:
		return false;
	}

	if (nullptr == HitRowArray || HitRowArray->Num() <= 0)
	{
		return false;
	}

	OutHitDatasPtr = HitRowArray;
	const int32 MaxIndexInclusive = HitRowArray->Num() - 1;
	OutClampedHitIndex = FMath::Clamp(HitIndexDesired, 0, MaxIndexInclusive);

	return true;
}

bool ACWeaponCombo::Combo_AttemptApplyHitAgainst(
	UPrimitiveComponent* WeaponPrimitive,
	UPrimitiveComponent* OtherPrim,
	ACCommonCharacter* HitCharacter,
	const FVector& InHitWorldLocation)
{
	(void)WeaponPrimitive;

	if (false == IsValid(Character) || false == IsValid(HitCharacter))
	{
		return false;
	}

	UCCharacterInfoComponent* OwnerInfoRaw = Character->FindComponentByClass<UCCharacterInfoComponent>();
	if (IsValid(OwnerInfoRaw) && OwnerInfoRaw->IsSameGroup(HitCharacter))
	{
		return false;
	}

	for (TObjectPtr<ACCommonCharacter>& ExistingWeak : ComboHitted)
	{
		if (ExistingWeak.Get() == HitCharacter)
		{
			return false;
		}
	}

	TArray<FHitData>* HitRows = nullptr;
	int32 HitIx = 0;
	if (false ==
		Combo_ResolveHitDatasForMelee(
			this,
			CurAttackType,
			ComboHitIndexCommon,
			ComboHitIndexAir,
			ComboHitIndexFlying,
			ComboHitIndexDown,
			ComboHitIndexSwaying,
			ComboHitIndexFallDown,
			ComboHitIndexSkill,
			HitRows,
			HitIx))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] HitData ?놁쓬 ?먮뒗 誘몄???AttackType=%d (%s): ???%s"),
			static_cast<int32>(CurAttackType),
			*GetNameSafe(this),
			*GetNameSafe(HitCharacter)));
		return false;
	}

	(void)OtherPrim;

	(*HitRows)[HitIx].SendDamage(TWeakObjectPtr<ACCommonCharacter>(Character), this, HitCharacter);
	ComboHitted.AddUnique(HitCharacter);
	Combo_OnBeginOverlap(HitCharacter, InHitWorldLocation);

	return true;
}

void ACWeaponCombo::ComboOnShapeBeginOverlap_Impl(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepFromMaybe)
{
	(void)OtherBodyIndex;
	(void)bFromSweep;
	(void)SweepFromMaybe;

	ACCommonCharacter* HitChar = Cast<ACCommonCharacter>(OtherActor);
	if (false == IsValid(HitChar))
	{
		return;
	}

	if (false == IsValid(Character))
	{
		return;
	}

	TArray<AActor*> ActorsIgnore;
	ActorsIgnore.Reserve(2);
	ActorsIgnore.Add(Character);

	FVector ImpactWorld =
		IsValid(OtherComp) ? OtherComp->K2_GetComponentLocation() : HitChar->GetActorLocation();

	bool TraceOk = UCYJJBlueprintLibrary::TryGetHitPointBetweenPrimitives(
		OverlappedComponent,
		OtherComp,
		ComboMeleeHitSphereRadius,
		ActorsIgnore,
		ImpactWorld);

	if (false == TraceOk)
	{
		if (true == IsValid(OtherComp))
		{
			ImpactWorld = OtherComp->K2_GetComponentLocation();
		}
		else
		{
			ImpactWorld = HitChar->GetActorLocation();
		}
	}

	Combo_AttemptApplyHitAgainst(OverlappedComponent, OtherComp, HitChar, ImpactWorld);
}

void ACWeaponCombo::ComboOnShapeHit_Impl(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	(void)NormalImpulse;

	ACCommonCharacter* HitChar = Cast<ACCommonCharacter>(OtherActor);
	if (false == IsValid(HitChar))
	{
		return;
	}

	FVector ImpactWorld = FVector::ZeroVector;
	if (true == Hit.bBlockingHit && false == Hit.ImpactPoint.Equals(FVector::ZeroVector))
	{
		ImpactWorld = Hit.ImpactPoint;
	}
	else if (true == IsValid(OtherComp))
	{
		ImpactWorld = OtherComp->K2_GetComponentLocation();
	}
	else
	{
		ImpactWorld = HitChar->GetActorLocation();
	}

	Combo_AttemptApplyHitAgainst(HitComponent, OtherComp, HitChar, ImpactWorld);
}

void ACWeaponCombo::Combo_OnBeginOverlap_Implementation(
	ACCommonCharacter* InOtherCharacter,
	FVector InHitPoint)
{
	(void)InHitPoint;

	if (false == IsValid(InOtherCharacter))
	{
		return;
	}

	// ?덇굅??BP Weapon_Combo:OnBeginOverlap ??SendDamage(Damaged) ?ㅼ쓬 CCProcess 濡??덊듃?됱쓽 CrowdControl ???곸슜?덈떎.
	// ?ш린?쒕뒗 TakeDamage 寃쎈줈?먯꽌 ?대? AttackType 湲곕컲 ?쇨꺽 ??낆쓣 ?ｌ? ?? CC 媛 None ???꾨땲硫??대떦 諛섏쓳?쇰줈 ??뼱?대떎.
	TArray<FHitData>* HitRows = nullptr;
	int32 HitIx = 0;
	if (false ==
		Combo_ResolveHitDatasForMelee(
			this,
			CurAttackType,
			ComboHitIndexCommon,
			ComboHitIndexAir,
			ComboHitIndexFlying,
			ComboHitIndexDown,
			ComboHitIndexSwaying,
			ComboHitIndexFallDown,
			ComboHitIndexSkill,
			HitRows,
			HitIx))
	{
		return;
	}

	const CECrowdControl CrowdControl = (*HitRows)[HitIx].CrowdControl;

	UCStateComponent* VictimState = InOtherCharacter->FindComponentByClass<UCStateComponent>();
	if (false == IsValid(VictimState))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Combo_OnBeginOverlap: ?쇨꺽?먯뿉 UCStateComponent ?놁쓬 ??%s"),
			*GetNameSafe(InOtherCharacter)));
		return;
	}

	switch (CrowdControl)
	{
	case CECrowdControl::None:
		break;
	case CECrowdControl::Air:
		VictimState->SetHitReactionAir();
		break;
	case CECrowdControl::PutDown:
		VictimState->SetHitFlyingPutDownMode();
		break;
	case CECrowdControl::Down:
		VictimState->SetHitDownMode();
		break;
	default:
		break;
	}
}

void ACWeaponCombo::Weapon_DoActionImpl(const CEAttackType InAttackType, const int32 InSkillIndex)
{
	CancelDashes();

	if (true == InAction && InAttackType != PrevAttackType)
	{
		const CEAttackType PrevForEnd = PrevAttackType;
		End_DoAction(PrevForEnd);
		PrevAttackType = InAttackType;
		InAction = true;
		CurAttackType = InAttackType;
		return;
	}

	InAction = true;
	CurAttackType = InAttackType;
	// ?덇굅??`Weapon` 洹몃옒?꾩? 媛숈씠 Common/Air/Flying ??`Begin_Do*` ?먯꽌 ?대룞쨌?곗튂 ?좏뻾(肄ㅻ낫??Air ?곗튂쨌Flying ??鍮??ы겕).
	Begin_DoAction(InAttackType);
	Combo_DispatchDoAction(InAttackType, InSkillIndex);
}

void ACWeaponCombo::Begin_DoAction_Implementation(const CEAttackType InAttackType)
{
	CurAttackType = InAttackType;

	switch (InAttackType)
	{
	case CEAttackType::Common:
		ComboFork_Begin_Common();
		break;
	case CEAttackType::Air:
		ComboFork_Begin_Air();
		break;
	case CEAttackType::Flying:
		ComboFork_Begin_Flying();
		break;
	default:
		// ?덇굅??Begin_DoAction ??Common/Air/Flying 留?Begin_* ???곌껐.
		break;
	}
}

void ACWeaponCombo::End_DoAction(const CEAttackType InAttackType)
{
	CurAttackType = InAttackType;

	switch (InAttackType)
	{
	case CEAttackType::Common:
		ComboFork_End_Common();
		break;
	case CEAttackType::Air:
		ComboFork_End_Air();
		break;
	case CEAttackType::Flying:
		ComboFork_End_Flying();
		break;
	case CEAttackType::Down:
		ComboFork_End_Down();
		break;
	case CEAttackType::DashAttack:
		ComboFork_End_DashAttack();
		break;
	case CEAttackType::FallDown:
		ComboFork_End_FallDown();
		break;
	case CEAttackType::Skill:
		ComboFork_End_Skill();
		break;
	case CEAttackType::Riding:
		ComboFork_End_Riding();
		break;
	default:
		// ?덇굅??End_DoAction: Dash / AirDash ?깆? 醫낅즺 遺꾧린 誘몄뿰寃?Riding ? `ComboFork_End_Riding` 泥섎━).
		break;
	}

	ACWeapon::End_DoAction(InAttackType);
}

bool ACWeaponCombo::Combo_ApplyLegacyDoAirComboAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoAirComboDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Air ??DoAction ???놁쓬 ??%s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponCombo] %s — StateComponent 없음"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsIdle() || StateComp->IsHitted();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	ACPlayableCharacter* Playable = Cast<ACPlayableCharacter>(Character);
	if (IsValid(Playable))
	{
		Playable->SetCoordBeforeAir();
		Playable->SetFlyToFall(false);
	}

	const int32 Idx = FMath::Clamp(ComboHitIndexAir, 0, DoAirComboDatas.Num() - 1);
	PlayAction(DoAirComboDatas, Idx, FVector::ZeroVector);
	return true;
}

bool ACWeaponCombo::Combo_ApplyLegacyDoFlyingAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoFlyingAttackDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Flying ??DoAction ???놁쓬 ??%s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponCombo] %s — StateComponent 없음"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsMovementHittedState() || StateComp->IsFlying()
		|| StateComp->IsFalling();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	ACharacter* CharActor = Cast<ACharacter>(Character);
	UCharacterMovementComponent* MoveComp =
		IsValid(CharActor) ? CharActor->GetCharacterMovement() : nullptr;
	if (false == IsValid(MoveComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s ??CharacterMovement ?놁쓬"),
			BranchLabelForLog));
		return false;
	}

	MoveComp->SetMovementMode(MOVE_Flying);

	const int32 Idx = FMath::Clamp(ComboHitIndexFlying, 0, DoFlyingAttackDatas.Num() - 1);
	PlayAction(DoFlyingAttackDatas, Idx, FVector::ZeroVector);
	return true;
}

void ACWeaponCombo::FallDownGravity()
{
	// BP: `Character->GetComponentByClass(MovingComponent)` ?좏슚 ?쒖뿉留?`SetGravity(20)`.
	UCMovementComponent* MoveResolved = Moving;
	if (false == IsValid(MoveResolved) && IsValid(Character))
	{
		MoveResolved = Character->FindComponentByClass<UCMovementComponent>();
	}

	if (false == IsValid(MoveResolved))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] FallDownGravity: MovingComponent 誘명빐寃???以묐젰 ?앸왂 ??%s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	MoveResolved->SetGravity(ComboFallDownGravityBpGravityScale);
}

bool ACWeaponCombo::Combo_ApplyLegacyDoDownAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoDownAttackDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Down ??DoAction ???놁쓬 ??%s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponCombo] %s — StateComponent 없음"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsIdle() || StateComp->IsHitted();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	const int32 Idx = FMath::Clamp(ComboHitIndexDown, 0, DoDownAttackDatas.Num() - 1);
	PlayAction(DoDownAttackDatas, Idx, FVector::ZeroVector);
	return true;
}

bool ACWeaponCombo::Combo_ApplyLegacyDoDashAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoSwayingAttackDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] DashAttack ??DoAction ???놁쓬 ??%s"),
			*GetNameSafe(this)));
		return false;
	}

	InGroundDashing = false;
	InAirDashing = false;

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponCombo] %s — StateComponent 없음"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsIdle() || StateComp->IsHitted();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	const int32 Idx = FMath::Clamp(ComboHitIndexSwaying, 0, DoSwayingAttackDatas.Num() - 1);
	PlayAction(DoSwayingAttackDatas, Idx, FVector::ZeroVector);
	return true;
}

bool ACWeaponCombo::Combo_ApplyLegacyDoFallDownAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoFallDownAttackDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] FallDown ??DoAction ???놁쓬 ??%s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponCombo] %s — StateComponent 없음"),
			BranchLabelForLog));
		return false;
	}

	const bool bAllowByState = StateComp->IsFalling() || StateComp->IsFlying();
	if (false == bAllowByState)
	{
		return false;
	}

	StateComp->SetAction();

	UWorld* World = GetWorld();
	if (false == IsValid(World))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s ??World ?놁뼱 FallDown ?癒??앸왂 ??%s"),
			BranchLabelForLog,
			*GetNameSafe(this)));
	}
	else
	{
		World->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
		World->GetTimerManager().SetTimer(
			ComboFallDownGravityTimerHandle,
			FTimerDelegate::CreateUObject(this, &ACWeaponCombo::FallDownGravity),
			ComboFallDownGravityDelaySeconds,
			false);
	}

	const int32 Idx = FMath::Clamp(ComboHitIndexFallDown, 0, DoFallDownAttackDatas.Num() - 1);
	PlayAction(DoFallDownAttackDatas, Idx, FVector::ZeroVector);
	return true;
}

bool ACWeaponCombo::Combo_ApplyLegacySkillAfterBeginGuards(const int32 InSkillIndex, const TCHAR* BranchLabelForLog)
{
	if (false == SkillDatas.IsValidIndex(InSkillIndex))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] Skill ???ㅽ궗 ???몃뜳??臾댄슚(Index=%d Count=%d) ??%s"),
			InSkillIndex,
			SkillDatas.Num(),
			*GetNameSafe(this)));
		return false;
	}

	ComboHitIndexSkill = InSkillIndex;

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponCombo] %s — StateComponent 없음"),
			BranchLabelForLog));
		return false;
	}

	const bool bRidingBranch = StateComp->IsRiding();

	const bool bTimedGravityBranch = StateComp->IsIdle() || StateComp->IsMovementHittedState()
		|| StateComp->IsFlying() || StateComp->IsFalling();

	if (false == bRidingBranch && false == bTimedGravityBranch)
	{
		return false;
	}

	StateComp->SetAction();

	UWorld* World = GetWorld();
	const int32 PlayIndex = FMath::Clamp(InSkillIndex, 0, SkillDatas.Num() - 1);

	if (true == bTimedGravityBranch)
	{
		if (false == IsValid(World))
		{
			CLog::Log(FString::Printf(
				TEXT("[WeaponComboSkillContext] %s ??World ?놁뼱 Skill??FallDownGravity ?癒??앸왂 ??%s"),
				BranchLabelForLog,
				*GetNameSafe(this)));
		}
		else
		{
			World->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
			World->GetTimerManager().SetTimer(
				ComboFallDownGravityTimerHandle,
				FTimerDelegate::CreateUObject(this, &ACWeaponCombo::FallDownGravity),
				ComboFallDownGravityDelaySeconds,
				false);
		}
	}

	PlayAction(SkillDatas, PlayIndex, FVector::ZeroVector);
	return true;
}

bool ACWeaponCombo::Combo_ApplyLegacyRidingAttackAfterBeginGuards(const TCHAR* BranchLabelForLog)
{
	if (DoActionDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] RidingAttack ??DoAction(怨듯넻) ???놁쓬 ??%s"),
			*GetNameSafe(this)));
		return false;
	}

	if (true == ComboEnable)
	{
		DisableCombo();
		ComboExist = true;
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (false == IsValid(StateComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponCombo] %s — StateComponent 없음"),
			BranchLabelForLog));
		return false;
	}

	if (false == StateComp->IsRiding())
	{
		return false;
	}

	StateComp->SetAction();

	const int32 Idx = FMath::Clamp(ComboHitIndexCommon, 0, DoActionDatas.Num() - 1);
	PlayAction(DoActionDatas, Idx, FVector::ZeroVector);
	return true;
}

void ACWeaponCombo::Combo_BeginFlyingSegmentFromNotify()
{
	if (false == InEquip || false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] BeginFlyingSegment: ?μ갑/罹먮┃???놁쓬 ??%s"),
			*GetNameSafe(this)));
		return;
	}

	if (true == ComboExist)
	{
		ComboExist = false;
	}

	ComboHitIndexFlying += 1;

	if (DoFlyingAttackDatas.Num() <= 0)
	{
		return;
	}

	// ?덇굅??BP `Less` 遺꾧린? ?숇벑: 利앸텇???몃뜳?ㅺ? ??諛곗뿴 湲몄씠 誘몃쭔???뚮쭔 以묐젰 蹂댁젙쨌?ъ깮(True).
	if (ComboHitIndexFlying >= DoFlyingAttackDatas.Num())
	{
		return;
	}

	if (IsValid(Moving))
	{
		Moving->AddGravity(ComboBeginFlyingNotifyAddGravityFactor);
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] BeginFlyingSegment: Moving ?놁뼱 AddGravity ?앸왂 ??%s"),
			*GetNameSafe(this)));
	}

	PlayAction(DoFlyingAttackDatas, ComboHitIndexFlying, FVector::ZeroVector);
}

void ACWeaponCombo::Combo_PlayDatasOrWarn(
	const TArray<FDoActionData>& InDatas,
	const int32 InPreferredIndex,
	const TCHAR* BranchLabelForLog)
{
	if (InDatas.Num() <= 0)
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] %s ??DoAction ???놁쓬 ??%s"),
			BranchLabelForLog,
			*GetNameSafe(this)));
		return;
	}

	const int32 ClampedIndex = FMath::Clamp(InPreferredIndex, 0, InDatas.Num() - 1);
	PlayAction(InDatas, ClampedIndex, FVector::ZeroVector);
}

void ACWeaponCombo::Combo_DispatchDoAction(const CEAttackType InAttackType, const int32 InSkillIndex)
{
	switch (InAttackType)
	{
	case CEAttackType::Common:
		Combo_PlayDatasOrWarn(DoActionDatas, 0, TEXT("Common"));
		break;
	case CEAttackType::Air:
		(void)Combo_ApplyLegacyDoAirComboAfterBeginGuards(TEXT("Air"));
		break;
	case CEAttackType::Flying:
		(void)Combo_ApplyLegacyDoFlyingAttackAfterBeginGuards(TEXT("Flying"));
		break;
	case CEAttackType::Down:
		(void)Combo_ApplyLegacyDoDownAttackAfterBeginGuards(TEXT("Down"));
		break;
	case CEAttackType::DashAttack:
		(void)Combo_ApplyLegacyDoDashAttackAfterBeginGuards(TEXT("DashAttack"));
		break;
	case CEAttackType::FallDown:
		(void)Combo_ApplyLegacyDoFallDownAttackAfterBeginGuards(TEXT("FallDown"));
		break;
	case CEAttackType::Dash:
	case CEAttackType::AirDash:
		// ?덇굅??Weapon_Combo DoAction ?ㅼ쐞移섏뿉?쒕뒗 誘몄뿰寃???吏硫는룰났以???쒕뒗 Dash()/AirDash() 媛 PlayAction 泥섎━.
		break;
	case CEAttackType::Skill:
		(void)Combo_ApplyLegacySkillAfterBeginGuards(InSkillIndex, TEXT("Skill"));
		break;
	case CEAttackType::Riding:
		(void)Combo_ApplyLegacyRidingAttackAfterBeginGuards(TEXT("RidingAttack"));
		break;
	case CEAttackType::Max:
	default:
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ?????녿뒗 AttackType=%d ??%s"),
			static_cast<int32>(InAttackType),
			*GetNameSafe(this)));
		break;
	}
}

void ACWeaponCombo::ComboFork_Begin_Common()
{
}

void ACWeaponCombo::ComboFork_Begin_Air()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_Begin_Air: Character ?놁쓬 ??%s"),
			*GetNameSafe(this)));
		return;
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	if (false == IsValid(CharActor))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_Begin_Air: ACharacter ?꾨떂 ??%s"),
			*GetNameSafe(Character)));
		return;
	}

	const FVector LaunchVelocity(0.0, 0.0, ComboBeginAirLaunchUp);
	CharActor->LaunchCharacter(LaunchVelocity, false, false);

	UCharacterMovementComponent* MoveComp = CharActor->GetCharacterMovement();
	if (IsValid(MoveComp))
	{
		MoveComp->SetMovementMode(MOVE_Falling);
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (IsValid(StateComp))
	{
		StateComp->SetFalling();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_Begin_Air: StateComponent ?놁쓬 ??%s"),
			*GetNameSafe(this)));
	}
}

void ACWeaponCombo::ComboFork_Begin_Flying()
{
	// ?덇굅??`Begin_DoFlyingAttack` 蹂몃Ц? ?좊떂 ?명떚 ?깆뿉??`Combo_BeginFlyingSegmentFromNotify` 濡?遺꾨━.
}

void ACWeaponCombo::ComboFork_End_Air()
{
	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
	}

	UCStateComponent* StateComp = Weapon_ResolveStateComponent();
	if (IsValid(StateComp))
	{
		StateComp->SetFalling();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Air: StateComponent ?놁쓬 ??%s"),
			*GetNameSafe(this)));
	}
}

void ACWeaponCombo::ComboFork_End_Common()
{
}

void ACWeaponCombo::ComboFork_End_Flying()
{
}

void ACWeaponCombo::ComboFork_End_Down()
{
	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Down: Moving ?놁뼱 UnFixCamera ?앸왂 ??%s"),
			*GetNameSafe(this)));
	}
}

void ACWeaponCombo::ComboFork_End_DashAttack()
{
	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_DashAttack: Moving ?놁뼱 UnFixCamera ?앸왂 ??%s"),
			*GetNameSafe(this)));
	}
}

void ACWeaponCombo::ComboFork_End_FallDown()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
	}

	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
		Moving->Move();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_FallDown: Moving ?놁뼱 UnFixCamera/Move ?앸왂 ??%s"),
			*GetNameSafe(this)));
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	UCharacterMovementComponent* MoveComp = IsValid(CharActor) ? CharActor->GetCharacterMovement() : nullptr;
	if (false == IsValid(MoveComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_FallDown: CharacterMovement 誘명빐寃???%s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	MoveComp->SetMovementMode(MOVE_Falling);
}

void ACWeaponCombo::ComboFork_End_Skill()
{
	// ?덇굅??`End_Skill` ????대㉧濡??⑥? `FallDownGravity` 媛 ?댄썑???곗?硫?以묐젰??瑗ъ씪 ???덉뼱 臾댄슚??
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboFallDownGravityTimerHandle);
	}

	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
		Moving->Move();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Skill: Moving ?놁뼱 UnFixCamera/Move ?앸왂 ??%s"),
			*GetNameSafe(this)));
	}

	ACharacter* CharActor = Cast<ACharacter>(Character);
	UCharacterMovementComponent* MoveComp = IsValid(CharActor) ? CharActor->GetCharacterMovement() : nullptr;
	if (false == IsValid(MoveComp))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Skill: CharacterMovement 誘명빐寃???%s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	MoveComp->SetMovementMode(MOVE_Falling);
}

void ACWeaponCombo::ComboFork_End_Riding()
{
	// ?덇굅??`End_RidingAttack` ???쇱씠??肄ㅻ낫 ?몃뜳?ㅻ? 0?쇰줈 珥덇린?뷀븳 ??移대찓?쇰쭔 蹂듦뎄?쒕떎.
	ComboHitIndexCommon = 0;

	if (IsValid(Moving))
	{
		Moving->UnFixCamera();
	}
	else
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboSkillContext] ComboFork_End_Riding: Moving ?놁뼱 UnFixCamera ?앸왂 ??%s"),
			*GetNameSafe(this)));
	}
}
