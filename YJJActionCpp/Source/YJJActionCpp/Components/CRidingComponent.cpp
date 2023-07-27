#include "Components/CRidingComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Commons/CGameMode.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"
#include "Characters/Animals/CAnimal_AI.h"
#include "Engine/Texture2D.h"
#include "AIController.h"
#include "Components/CMovementComponent.h"
#include "Components/CStateComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/CEquipment.h"
#include "GameFramework/Controller.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Weapons/CAct.h"

UCRidingComponent::UCRidingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACAnimal_AI>(GetOwner());

	eMoveAction = EMoveComponentAction::Type::Move;
	latentInfo.CallbackTarget = this;

	YJJHelpers::GetAsset<UTexture2D>(&InteractionKeyTexture,
		TEXT("Texture2D'/Game/Assets/Textures/ButtonPrompts/F_Key_Dark.F_Key_Dark'"));

	YJJHelpers::GetAsset<UAnimMontage>(&MountAnims[static_cast<uint8>(CEDirection::Left)],
		TEXT("AnimMontage'/Game/Character/Player/Montages/Riding/Rider_Mount_Front_Left_Montage.Rider_Mount_Front_Left_Montage'"));

	YJJHelpers::GetAsset<UAnimMontage>(&MountAnims[static_cast<uint8>(CEDirection::Right)],
		TEXT("AnimMontage'/Game/Character/Player/Montages/Riding/Rider_Mount_Front_Right_Montage.Rider_Mount_Front_Right_Montage'"));

	YJJHelpers::GetAsset<UAnimMontage>(&MountAnims[static_cast<uint8>(CEDirection::Back)],
		TEXT("AnimMontage'/Game/Character/Player/Montages/Riding/Rider_Mount_Back_Montage.Rider_Mount_Back_Montage'"));

	YJJHelpers::GetAsset<USoundBase>(&MountSound,
		TEXT("SoundWave'/Game/Assets/Sounds/Action/Sway_2.Sway_2'"));

	YJJHelpers::GetAsset<UAnimMontage>(&UnmountAnim,
		TEXT("AnimMontage'/Game/Character/Player/Montages/Riding/Rider_Dismount_Front_Right_Montage.Rider_Dismount_Front_Right_Montage'"));

	YJJHelpers::GetAsset<USoundBase>(&UnmountSound,
		TEXT("SoundWave'/Game/Assets/Sounds/Action/Sway_2.Sway_2'"));
}

void UCRidingComponent::BeginPlay()
{
	Super::BeginPlay();

	CheckNull(Owner);

	Mesh = Owner->GetMesh();
	SpringArm = Owner->GetSpringArm();
	Camera = Owner->GetCamera();
	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
	MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());
	CamComp = YJJHelpers::GetComponent<UCCamComponent>(Owner.Get());

	RidingPoints[static_cast<uint8>(CERidingPoint::CandidateLeft)] = Owner->GetMountLeftPoint();
	RidingPoints[static_cast<uint8>(CERidingPoint::CandidateRight)] = Owner->GetMountRightPoint();
	RidingPoints[static_cast<uint8>(CERidingPoint::CandidateBack)] = Owner->GetMountBackPoint();
	RidingPoints[static_cast<uint8>(CERidingPoint::Rider)] = Owner->GetRiderPoint();
	RidingPoints[static_cast<uint8>(CERidingPoint::Unmount)] = Owner->GetUnmountPoint();

	InteractionCollision = Owner->GetInteractionCollision();

	if (CamComp.IsValid())
		CamComp->DisableControlRotation();

	if (SpringArm.IsValid())
		Zooming = SpringArm->TargetArmLength;
	// TODO Rider Info


	const TWeakObjectPtr<ACGameMode> gameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	Hud = Cast<UCUserWidget_HUD>(gameMode->GetHUD());

	CheckNull(Hud);

	Hud->SetChildren();
	Interaction = Hud->Interaction;

	InteractionText = FText::FromString(TEXT("탑승"));

	Interaction->SetChildren(InteractionKeyTexture, InteractionText);
}

void UCRidingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (RidingState)
	{
	case CERidingState::None:
		if (Rider.IsValid())
			SetRidingState(CERidingState::ToMountPoint);
		break;
	case CERidingState::ToMountPoint:
		Tick_ToMountPoint();
		break;
	case CERidingState::Mounting:
		Tick_Mounting();
		break;
	case CERidingState::MountingEnd:
		Tick_MountingEnd();
		break;
	case CERidingState::Riding:
		Tick_Riding();
		break;
	case CERidingState::Unmounting:
		Tick_Unmounting();
		break;
	case CERidingState::UnmountingEnd:
		Tick_UnmountingEnd();
		break;
	}
}

void UCRidingComponent::SetRidingState(const CERidingState InState)
{
	RidingState = InState;
}

void UCRidingComponent::BeginOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	CheckNull(Interaction);

	const TWeakObjectPtr<ACAnimal_AI> animal = Cast<ACAnimal_AI>(Owner);
	CheckNull(animal);

	const TWeakObjectPtr<ACPlayableCharacter> interactor = Cast<ACPlayableCharacter>(OtherActor);
	CheckNull(interactor);

	SetInteractor(interactor, Owner);
	SetInteractor(Owner, interactor);

	if (false == interactor->OnMount.IsBound())
		interactor->OnMount.AddUniqueDynamic(this, &UCRidingComponent::SetRider);

	Interaction->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCRidingComponent::EndOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex)
{
	CheckNull(Interaction);

	const TWeakObjectPtr<ACAnimal_AI> animal = Cast<ACAnimal_AI>(Owner);
	CheckNull(animal);

	const TWeakObjectPtr<ACPlayableCharacter> interactor = Cast<ACPlayableCharacter>(OtherActor);
	CheckNull(interactor);

	SetInteractor(interactor, nullptr);
	SetInteractor(Owner, nullptr);

	if (true == interactor->OnMount.IsBound())
		interactor->OnMount.RemoveDynamic(this, &UCRidingComponent::SetRider);

	Interaction->SetVisibility(ESlateVisibility::Collapsed);
}

void UCRidingComponent::SetInteractor(
	TWeakObjectPtr<ACCommonCharacter> InCharacter, 
	const TWeakObjectPtr<ACCommonCharacter> InOtherCharacter)
{
	InCharacter->SetInteractor(InOtherCharacter.Get());
}

void UCRidingComponent::SetRider(ACCommonCharacter* InCharacter)
{
	Rider = InCharacter;
	RiderStateComp = YJJHelpers::GetComponent<UCStateComponent>(InCharacter);
	RiderMovementComp = YJJHelpers::GetComponent<UCMovementComponent>(InCharacter);
	RiderCamComp = YJJHelpers::GetComponent<UCCamComponent>(InCharacter);
	RiderWeaponComp = YJJHelpers::GetComponent<UCWeaponComponent>(InCharacter);

	if (false == Owner->OnUnmount.IsBound())
		Owner->OnUnmount.AddDynamic(this, &UCRidingComponent::Unmount);
}

void UCRidingComponent::Tick_ToMountPoint()
{
	if (nullptr == RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)])
	{
		CheckValidPoint();
	}
	else if (true == MoveToPoint(Rider, RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)]))
	{
		Rider->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		Rider->GetCharacterMovement()->bEnablePhysicsInteraction = false;

		PossessAndInterpToCamera();

		SetRidingState(CERidingState::Mounting);
	}
}

void UCRidingComponent::CheckValidPoint()
{
	const float candidateLeft = UKismetMathLibrary::Vector_DistanceSquared(
		Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(CERidingPoint::CandidateLeft)]->GetComponentLocation());

	const float candidateRight = UKismetMathLibrary::Vector_DistanceSquared(
		Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(CERidingPoint::CandidateRight)]->GetComponentLocation());

	const float candidateBack = UKismetMathLibrary::Vector_DistanceSquared(
		Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(CERidingPoint::CandidateBack)]->GetComponentLocation());

	const float minCandidate = UKismetMathLibrary::Min(
		candidateLeft, UKismetMathLibrary::Min(
			candidateRight, 
			candidateBack));

	constexpr float nearFactor = 50;

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateLeft, nearFactor))
	{
		RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(CERidingPoint::CandidateLeft)];

		MountDirection = CEDirection::Left;

		CurMountRotationZFactor = MountRotationZFactor;
	}
	else if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateRight, nearFactor))
	{
		RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(CERidingPoint::CandidateRight)];

		MountDirection = CEDirection::Right;

		CurMountRotationZFactor = -MountRotationZFactor;
	}
	else if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateBack, nearFactor))
	{
		RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(CERidingPoint::CandidateBack)];

		MountDirection = CEDirection::Back;

		CurMountRotationZFactor = 0;
	}
}

bool UCRidingComponent::MoveToPoint(
	TWeakObjectPtr<ACCommonCharacter> Char, 
	const TWeakObjectPtr<USceneComponent> To)
{
	bool reached = true;
	constexpr float nearFactor = 15;
	bool bCheck = true;
	bCheck &= UKismetMathLibrary::NearlyEqual_FloatFloat(Char->GetActorLocation().X, To->GetComponentLocation().X, nearFactor);
	bCheck &= UKismetMathLibrary::NearlyEqual_FloatFloat(Char->GetActorLocation().Y, To->GetComponentLocation().Y, nearFactor);

	if (bCheck)
	{
		Rider->GetCharacterMovement()->StopMovementImmediately();
		RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)] = nullptr;
		reached = true;
	}
	else
	{
		const FVector unitDirection = 
			UKismetMathLibrary::GetDirectionUnitVector(Char->GetActorLocation(), To->GetComponentLocation());

		constexpr float scaleValue = 2;
		Rider->AddMovementInput(unitDirection, scaleValue);
		reached = false;
	}

	return reached;
}

void UCRidingComponent::PossessAndInterpToCamera()
{
	// Camera Start
	Camera->SetWorldTransform(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetTransform());

	// Controller Possess
	ControllerSave = Owner->GetController();

	const TWeakObjectPtr<ACCommonCharacter> animal = Cast<ACCommonCharacter>(Owner);

	GetWorld()->GetFirstPlayerController()->UnPossess();

	if (animal.IsValid())
		GetWorld()->GetFirstPlayerController()->Possess(animal.Get());

	Rider->SetMyCurController(GetWorld()->GetFirstPlayerController());

	// Camera Move
	latentInfo.CallbackTarget = Camera.Get();

	UKismetSystemLibrary::MoveComponentTo(
		Camera.Get(),
		FVector::ZeroVector, 
		FRotator::ZeroRotator,
		true, 
		true, 
		OverTime_Camera,
		false,
		eMoveAction, 
		latentInfo);
}

void UCRidingComponent::Tick_Mounting()
{
	FVector targetPos = RidingPoints[static_cast<uint8>(CERidingPoint::Rider)]->GetComponentLocation();
	targetPos.Z += MountZFactor;

	FRotator targetRot = RidingPoints[static_cast<uint8>(CERidingPoint::Rider)]->GetComponentRotation();
	targetRot = FRotator(0, targetRot.Yaw + CurMountRotationZFactor, 0);

	latentInfo.CallbackTarget = Rider.Get();

	// 탑승 중간 지점 위치, 방향으로
	UKismetSystemLibrary::MoveComponentTo(
		Rider->GetRootComponent(), 
		targetPos, 
		targetRot,
		true, 
		true, 
		OverTime_Mount,
		false,
		eMoveAction, 
		latentInfo);

	Rider->PlayAnimMontage(MountAnims[static_cast<uint8>(MountDirection)], MountAnimPlayRate);

	CheckNull(Rider->GetMesh());
	CheckNull(Rider->GetMesh()->GetAnimInstance());

	// 탑승중애니 - 탑승후루프애니
	// 몽타주 블렌드 아웃 되는 시점에 정확히
	Rider->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.AddUniqueDynamic(
		this, &UCRidingComponent::InterpToRiderPos);

	// Attach - 몽타주 끝나는 시점에 정확히
	Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.AddUniqueDynamic(
		this, &UCRidingComponent::AttachToRiderPoint);


	UGameplayStatics::PlaySoundAtLocation(this, MountSound, Owner->GetActorLocation());

	SetRidingState(CERidingState::MountingEnd);
}

void UCRidingComponent::InterpToRiderPos(UAnimMontage* Anim, bool bInterrupted)
{
	const FVector riderPos = RidingPoints[static_cast<uint8>(CERidingPoint::Rider)]->GetComponentLocation();
	const FRotator riderRot = RidingPoints[static_cast<uint8>(CERidingPoint::Rider)]->GetComponentRotation();

	// 실제 탑승 후 위치, 방향으로
	UKismetSystemLibrary::MoveComponentTo(
		Rider->GetRootComponent(), 
		riderPos, 
		riderRot,
		true, 
		true, 
		OverTime_RiderPos,
		false,
		eMoveAction, 
		latentInfo);

	if (true == Rider->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.IsBound())
		Rider->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.RemoveDynamic(
			this, &UCRidingComponent::InterpToRiderPos);
}

void UCRidingComponent::AttachToRiderPoint(UAnimMontage* Anim, bool bInterrupted)
{
	Rider->GetCharacterMovement()->StopMovementImmediately();

	Rider->AttachToComponent(Mesh.Get(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, "Rider");

	CheckNull(Rider->GetMesh());
	CheckNull(Rider->GetMesh()->GetAnimInstance());

	if (true == Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.IsBound())
		Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.RemoveDynamic(
			this, &UCRidingComponent::AttachToRiderPoint);
}

void UCRidingComponent::Tick_MountingEnd()
{
	// 애니 끝날 때까지 대기 후 실행
	CheckFalse(IsValid(Rider->GetCurrentMontage()));

	Rider->SetbRiding(true);
	Owner->SetbRiding(true);

	Rider->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	Rider->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	MovementComp->Move();
	MovementComp->InputAction_Run();

	SpringArm->bUsePawnControlRotation = true;

	Rider->SetLegIKAlpha(LegIKAlpha);


	// TODO Riding Info
	SetStatusUI();
	OnStatusUI(true);


	SetRidingState(CERidingState::Riding);
}

void UCRidingComponent::Tick_Riding() const
{
	CheckNull(CamComp);

	CheckNull(RiderWeaponComp);
	CheckNull(RiderWeaponComp->GetEquipment());

	if (true == *RiderWeaponComp->GetEquipment()->GetEquipped())
		CamComp->EnableControlRotation();
	else
		CamComp->DisableControlRotation();

	CheckNull(RiderCamComp);

	if (true == RiderCamComp->GetFixedCamera())
		CamComp->EnableFixedCamera();
	else
		CamComp->DisableFixedCamera();

	// TODO Height UI
}

void UCRidingComponent::Unmount()
{
	RidingState = CERidingState::Unmounting;

	if (true == Owner->OnUnmount.IsBound())
		Owner->OnUnmount.RemoveDynamic(this, &UCRidingComponent::Unmount);
}

void UCRidingComponent::Tick_Unmounting()
{
	CheckNull(Owner);
	CheckNull(Rider);

	Owner->GetCharacterMovement()->StopMovementImmediately();

	Rider->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	Rider->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Rider->GetCharacterMovement()->bEnablePhysicsInteraction = true;

	Rider->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

	Rider->SetActorLocation(FVector(
		Rider->GetActorLocation().X, 
		Rider->GetActorLocation().Y, 
		Rider->GetActorLocation().Z + UnmountZFactor));

	Rider->SetActorRotation(FRotator(0, 0, 0));

	Rider->PlayAnimMontage(UnmountAnim, UnmountAnimPlayRate);


	UnpossessAndInterpToCamera();

	// 탑승 후 위치, 방향으로
	const FVector targetPos = RidingPoints[static_cast<uint8>(CERidingPoint::Unmount)]->GetComponentLocation();
	//targetPos.Z += MountZFactor;
	FRotator targetRot = RidingPoints[static_cast<uint8>(CERidingPoint::Unmount)]->GetComponentRotation();
	targetRot = FRotator(0, targetRot.Yaw + CurMountRotationZFactor, 0);

	latentInfo.CallbackTarget = Rider.Get();

	UKismetSystemLibrary::MoveComponentTo(
		Rider->GetRootComponent(), 
		targetPos, 
		targetRot,
		true, 
		true, 
		OverTime_Unmount,
		false,
		eMoveAction, 
		latentInfo);

	if (RiderMovementComp.IsValid())
	{
		RiderMovementComp->Stop();
		RiderMovementComp->Right = 0.0f;
		RiderMovementComp->Forward = 0.0f;
	}

	if (RiderStateComp.IsValid())
		RiderStateComp->SetFallMode();

	Owner->SetbRiding(false);
	Rider->SetbRiding(false);


	// TODO Riding Info
	SetStatusUI();
	OnStatusUI(false);

	SetRidingState(CERidingState::UnmountingEnd);
}

void UCRidingComponent::UnpossessAndInterpToCamera()
{
	// Camera Start
	Camera->SetWorldTransform(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetTransform());

	// Controller Possess
	TWeakObjectPtr<ACCommonCharacter> animal = Cast<ACCommonCharacter>(Owner);

	GetWorld()->GetFirstPlayerController()->UnPossess();

	if (Rider.IsValid())
		GetWorld()->GetFirstPlayerController()->Possess(Rider.Get());

	Rider->SetMyCurController(GetWorld()->GetFirstPlayerController());

	Owner->Controller = ControllerSave;

	// Camera Move
	latentInfo.CallbackTarget = Camera.Get();

	UKismetSystemLibrary::MoveComponentTo(
		Camera.Get(),
		FVector::ZeroVector, 
		FRotator::ZeroRotator,
		true, 
		true, 
		OverTime_Camera,
		false,
		eMoveAction, 
		latentInfo);
}

void UCRidingComponent::Tick_UnmountingEnd()
{
	// 애니 끝날 때까지 대기 후 실행
	if (false == IsValid(Rider->GetCurrentMontage()))
	{
		SetInteractor(Owner, Rider);
		SetInteractor(Rider, Owner);

		RiderMovementComp->Move();

		Rider = nullptr;
		RiderStateComp = nullptr;
		RiderMovementComp = nullptr;
		RiderCamComp = nullptr;
		RiderWeaponComp = nullptr;

		SetRidingState(CERidingState::None);
	}
}

void UCRidingComponent::InputAction_Act()
{
	CheckNull(RiderWeaponComp);
	CheckNull(RiderWeaponComp->GetAct());

	RiderWeaponComp->GetAct()->Act();
}