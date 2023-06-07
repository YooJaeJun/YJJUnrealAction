#include "Components/CRidingComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Game/CGameMode.h"
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

UCRidingComponent::UCRidingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACAnimal_AI>(GetOwner());

	eMoveAction = EMoveComponentAction::Type::Move;
	latentInfo.CallbackTarget = this;
}

void UCRidingComponent::BeginPlay()
{
	Super::BeginPlay();

	const TWeakObjectPtr<ACGameMode> gameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	Hud = Cast<UCUserWidget_HUD>(gameMode->GetHUD());

	if (!!Hud)
	{
		Hud->SetChildren();
		Interaction = Hud->Interaction;

		CHelpers::LoadAsset<UTexture2D>(&InteractionKeyTexture,
			TEXT("Texture2D'/Game/Assets/Textures/ButtonPrompts/F_Key_Dark.F_Key_Dark'"));

		InteractionText = FText::FromString(TEXT("탑승"));

		Interaction->SetChildren(InteractionKeyTexture, InteractionText);
	}

	if (!!Owner.Get())
	{
		Mesh = Owner->GetMesh();
		SpringArm = Owner->GetSpringArm();
		Camera = Owner->GetCamera();
		MovementComp = Owner->MovementComp;
		StateComp = Owner->StateComp;

		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateLeft)] = Owner->GetMountLeftPoint();
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateRight)] = Owner->GetMountRightPoint();
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateBack)] = Owner->GetMountBackPoint();
		RidingPoints[static_cast<uint8>(ERidingPoint::Rider)] = Owner->GetRiderPoint();
		RidingPoints[static_cast<uint8>(ERidingPoint::Unmount)] = Owner->GetUnmountPoint();

		InteractionCollision = Owner->GetInteractionCollision();

		if (!!MovementComp)
			MovementComp->DisableControlRotation();

		if (!!SpringArm)
			Zooming = SpringArm->TargetArmLength;

		CHelpers::LoadAsset<USoundBase>(&MountSound,
			TEXT("SoundWave'/Game/Assets/Sounds/Action/Sway_2.Sway_2'"));

		CHelpers::LoadAsset<UAnimMontage>(&UnmountAnim,
			TEXT("AnimMontage'/Game/Character/AnimationMontages/Riding/Rider_Dismount_Front_Right_Montage.Rider_Dismount_Front_Right_Montage'"));

		CHelpers::LoadAsset<USoundBase>(&UnmountSound,
			TEXT("SoundWave'/Game/Assets/Sounds/Action/Sway_2.Sway_2'"));

		// TODO Eye
		// TODO Rider Info
	}
}

void UCRidingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	switch (RidingState)
	{
	case ERidingState::None:
		if (!!Rider)
			SetRidingState(ERidingState::ToMountPoint);
		break;
	case ERidingState::ToMountPoint:
		Tick_ToMountPoint();
		break;
	case ERidingState::Mounting:
		Tick_Mounting();
		break;
	case ERidingState::MountingEnd:
		Tick_MountingEnd();
		break;
	case ERidingState::Riding:
		Tick_Riding();
		break;
	case ERidingState::Unmounting:
		Tick_Unmounting();
		break;
	case ERidingState::UnmountingEnd:
		Tick_UnmountingEnd();
		break;
	}
}

void UCRidingComponent::SetRidingState(const ERidingState InState)
{
	RidingState = InState;
}

void UCRidingComponent::BeginOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CheckNull(Interaction);

	const auto animal = Cast<ACAnimal_AI>(OtherActor);
	CheckTrue(!!animal);

	const auto interactor = Cast<ACPlayableCharacter>(OtherActor);
	CheckNull(interactor);

	SetInteractor(interactor, Owner);
	SetInteractor(Owner, interactor);

	if (false == interactor->OnMount.IsBound())
		interactor->OnMount.AddDynamic(this, &UCRidingComponent::SetRider);

	Interaction->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCRidingComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CheckNull(Interaction);

	const auto animal = Cast<ACAnimal_AI>(OtherActor);
	CheckTrue(!!animal);

	const auto interactor = Cast<ACPlayableCharacter>(OtherActor);
	CheckNull(interactor);

	SetInteractor(interactor, nullptr);
	SetInteractor(Owner, nullptr);

	if (!!interactor->OnMount.IsBound())
		interactor->OnMount.RemoveDynamic(this, &UCRidingComponent::SetRider);

	Interaction->SetVisibility(ESlateVisibility::Collapsed);
}

void UCRidingComponent::SetInteractor(TWeakObjectPtr<ACCommonCharacter> InCharacter, 
	const TWeakObjectPtr<ACCommonCharacter> InOtherCharacter)
{
	InCharacter->SetInteractor(InOtherCharacter.Get());
}

void UCRidingComponent::SetRider(ACCommonCharacter* InCharacter)
{
	Rider = InCharacter;
	RiderWeaponComp = CHelpers::GetComponent<UCWeaponComponent>(InCharacter);

	if (false == Owner->OnUnmount.IsBound())
		Owner->OnUnmount.AddDynamic(this, &UCRidingComponent::Unmount);
}

void UCRidingComponent::Tick_ToMountPoint()
{
	if (nullptr == RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)])
		CheckValidPoint();
	else if (MoveToPoint(Rider, RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)]))
	{
		Rider->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
		Rider->GetCharacterMovement()->bEnablePhysicsInteraction = false;


		PossessAndInterpToCamera();


		SetRidingState(ERidingState::Mounting);
	}
}

void UCRidingComponent::CheckValidPoint()
{
	const float candidateLeft = UKismetMathLibrary::Vector_DistanceSquared(Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateLeft)]->GetComponentLocation());

	const float candidateRight = UKismetMathLibrary::Vector_DistanceSquared(Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateRight)]->GetComponentLocation());

	const float candidateBack = UKismetMathLibrary::Vector_DistanceSquared(Rider->GetActorLocation(),
		RidingPoints[static_cast<uint8>(ERidingPoint::CandidateBack)]->GetComponentLocation());

	const float minCandidate = UKismetMathLibrary::Min(candidateLeft, UKismetMathLibrary::Min(candidateRight, candidateBack));

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateLeft, 5.0f))
	{
		MountDir = EDirection::Left;
		RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(ERidingPoint::CandidateLeft)];

		CHelpers::LoadAsset<UAnimMontage>(&MountAnim,
			TEXT("AnimMontage'/Game/Character/AnimationMontages/Riding/Rider_Mount_Front_Left_Montage.Rider_Mount_Front_Left_Montage'"));

		MountRotationZFactor = 100.0f;
	}
	else if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateRight, 5.0f))
	{
		MountDir = EDirection::Right;
		RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(ERidingPoint::CandidateRight)];

		CHelpers::LoadAsset<UAnimMontage>(&MountAnim,
			TEXT("AnimMontage'/Game/Character/AnimationMontages/Riding/Rider_Mount_Front_Right_Montage.Rider_Mount_Front_Right_Montage'"));

		MountRotationZFactor = -100.0f;
	}
	else if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateBack, 5.0f))
	{
		MountDir = EDirection::Back;
		RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(ERidingPoint::CandidateBack)];

		CHelpers::LoadAsset<UAnimMontage>(&MountAnim,
			TEXT("AnimMontage'/Game/Character/AnimationMontages/Riding/Rider_Mount_Back_Montage.Rider_Mount_Back_Montage'"));

		MountRotationZFactor = 0.0f;
	}
}

bool UCRidingComponent::MoveToPoint(ACCommonCharacter* Char, const USceneComponent* To)
{
	bool reached = true;

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(Char->GetActorLocation().X, To->GetComponentLocation().X, 15.0f) &&
		UKismetMathLibrary::NearlyEqual_FloatFloat(Char->GetActorLocation().Y, To->GetComponentLocation().Y, 15.0f))
	{
		Rider->GetCharacterMovement()->StopMovementImmediately();
		RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)] = nullptr;
		reached = true;
	}
	else
	{
		const FVector unitDirection = UKismetMathLibrary::GetDirectionUnitVector(Char->GetActorLocation(), To->GetComponentLocation());
		Rider->AddMovementInput(unitDirection, 1.0f);
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

	TWeakObjectPtr<ACCommonCharacter> animal = Cast<ACCommonCharacter>(Owner);

	GetWorld()->GetFirstPlayerController()->UnPossess();

	if (!!animal.Get())
		GetWorld()->GetFirstPlayerController()->Possess(animal.Get());

	Rider->SetMyCurController(GetWorld()->GetFirstPlayerController());

	// Camera Move
	latentInfo.CallbackTarget = Camera;

	UKismetSystemLibrary::MoveComponentTo(Camera,
		FVector::ZeroVector, FRotator::ZeroRotator,
		true, true, 0.7f, false,
		eMoveAction, latentInfo);
}


void UCRidingComponent::Tick_Mounting()
{
	// 탑승 후 위치, 방향으로
	FVector targetPos = RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentLocation();
	targetPos.Z += 20.0f;
	FRotator targetRot = RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentRotation();
	targetRot = FRotator(0, targetRot.Yaw + MountRotationZFactor, 0);

	latentInfo.CallbackTarget = Rider;

	UKismetSystemLibrary::MoveComponentTo(Rider->GetRootComponent(), targetPos, targetRot,
		true, true, 0.3f, false,
		eMoveAction, latentInfo);


	Rider->PlayAnimMontage(MountAnim, 1.5f);


	// 탑승중애니 - 탑승후루프애니
	// MoveComponentTo - 몽타주 블렌드 아웃 되는 시점에 딱
	if (false == Rider->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.IsBound())
		Rider->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(
			this, &UCRidingComponent::InterpToRidingPos);

	// Attach - 몽타주 끝나는 시점에 딱
	if (false == Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.IsBound())
		Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(
			this, &UCRidingComponent::AttachToRiderPoint);


	UGameplayStatics::PlaySoundAtLocation(this, MountSound, Owner->GetActorLocation());

	SetRidingState(ERidingState::MountingEnd);
}

void UCRidingComponent::InterpToRidingPos(UAnimMontage* Anim, bool bInterrupted)
{
	const FVector ridingPos = RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentLocation();
	const FRotator ridingRot = RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentRotation();

	UKismetSystemLibrary::MoveComponentTo(Rider->GetRootComponent(), ridingPos, ridingRot,
		true, true, 0.2f, false,
		eMoveAction, latentInfo);

	if (!!Rider->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.IsBound())
		Rider->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.RemoveDynamic(
			this, &UCRidingComponent::InterpToRidingPos);
}

void UCRidingComponent::AttachToRiderPoint(UAnimMontage* Anim, bool bInterrupted)
{
	Rider->GetCharacterMovement()->StopMovementImmediately();

	Rider->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "RiderPoint");

	if (!!Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.IsBound())
		Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.RemoveDynamic(
			this, &UCRidingComponent::AttachToRiderPoint);
}

void UCRidingComponent::Tick_MountingEnd()
{
	// 애니 끝날 때까지 대기 후 실행
	if (false == IsValid(Rider->GetCurrentMontage()))
	{
		Rider->SetbRiding(true);
		Owner->SetbRiding(true);

		Rider->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		Rider->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		MovementComp->Move();

		SpringArm->bUsePawnControlRotation = true;


		// TODO Riding Info
		SetStatusUI();
		OnStatusUI(true);


		SetRidingState(ERidingState::Riding);
	}
}

void UCRidingComponent::Tick_Riding()
{
	if (!!RiderWeaponComp->GetEquipment() &&
		!!RiderWeaponComp->GetEquipment()->GetEquipped())
		MovementComp->EnableControlRotation();
	else
		MovementComp->DisableControlRotation();

	if (!!Rider->MovementComp->GetFixedCamera())
		MovementComp->FixCamera();
	else
		MovementComp->UnFixCamera();

	// TODO Zoom
	// TODO Height UI
}

void UCRidingComponent::Unmount()
{
	RidingState = ERidingState::Unmounting;

	if (!!Owner->OnUnmount.IsBound())
		Owner->OnUnmount.RemoveDynamic(this, &UCRidingComponent::Unmount);
}

void UCRidingComponent::Tick_Unmounting()
{
	Owner->GetCharacterMovement()->StopMovementImmediately();

	Rider->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	Rider->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Rider->GetCharacterMovement()->bEnablePhysicsInteraction = true;

	Rider->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);

	Rider->SetActorLocation(FVector(Rider->GetActorLocation().X, Rider->GetActorLocation().Y, 
		Rider->GetActorLocation().Z + 30.0f));

	Rider->SetActorRotation(FRotator(0, 0, 0));

	Rider->PlayAnimMontage(UnmountAnim, 2.0f);


	UnpossessAndInterpToCamera();

	// 탑승 후 위치, 방향으로
	FVector targetPos = RidingPoints[static_cast<uint8>(ERidingPoint::Unmount)]->GetComponentLocation();
	targetPos.Z += 20.0f;
	FRotator targetRot = RidingPoints[static_cast<uint8>(ERidingPoint::Unmount)]->GetComponentRotation();
	targetRot = FRotator(0, targetRot.Yaw + MountRotationZFactor, 0);

	latentInfo.CallbackTarget = Rider;

	UKismetSystemLibrary::MoveComponentTo(Rider->GetRootComponent(), targetPos, targetRot,
		true, true, 0.6f, false,
		eMoveAction, latentInfo);

	Owner->SetbRiding(false);
	Rider->SetbRiding(false);
	Rider->StateComp->SetFallMode();
	Rider->StateComp->SetFallMode();


	// TODO Riding Info
	SetStatusUI();
	OnStatusUI(false);

	SetRidingState(ERidingState::UnmountingEnd);
}

void UCRidingComponent::UnpossessAndInterpToCamera()
{
	// Camera Start
	Camera->SetWorldTransform(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetTransform());

	// Controller Possess
	TWeakObjectPtr<ACCommonCharacter> animal = Cast<ACCommonCharacter>(Owner);

	GetWorld()->GetFirstPlayerController()->UnPossess();

	if (!!Rider)
		GetWorld()->GetFirstPlayerController()->Possess(Rider);

	Rider->SetMyCurController(GetWorld()->GetFirstPlayerController());

	Owner->Controller = ControllerSave;

	// Camera Move
	latentInfo.CallbackTarget = Camera;

	UKismetSystemLibrary::MoveComponentTo(Camera,
		FVector::ZeroVector, FRotator::ZeroRotator,
		true, true, 0.7f, false,
		eMoveAction, latentInfo);
}

void UCRidingComponent::Tick_UnmountingEnd()
{
	// 애니 끝날 때까지 대기 후 실행
	if (false == IsValid(Rider->GetCurrentMontage()))
	{
		SetInteractor(Owner, Rider);
		SetInteractor(Rider, Owner);

		Rider = nullptr;
		RiderWeaponComp = nullptr;

		SetRidingState(ERidingState::None);
	}
}
