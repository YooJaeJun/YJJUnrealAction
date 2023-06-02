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
}

void UCRidingComponent::BeginPlay()
{
	Super::BeginPlay();

	TWeakObjectPtr<ACGameMode> gameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
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

		// TODO Eye
		// TODO Rider Info

		//if (false == Owner->GetRiderPoint()->AttachToComponent(Owner->GetMesh(),
		//	FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		//	"RiderPoint"))
		//	CLog::Log("RiderPoint Attach Fail");
	}
}

void UCRidingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (RidingState)
	{
	case ERidingState::None:
		if (!!Rider)
			RidingState = ERidingState::ToMountPoint;
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

void UCRidingComponent::BeginOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CheckNull(Interaction);

	const auto animal = Cast<ACAnimal_AI>(OtherActor);
	CheckTrue(!!animal);

	const auto interactor = Cast<ACCommonCharacter>(OtherActor);
	CheckNull(interactor);

	SetInteractableCharacter(interactor, Owner);
	SetInteractableCharacter(Owner, interactor);

	if (false == interactor->OnInteract.IsBound())
		interactor->OnInteract.AddDynamic(this, &UCRidingComponent::SetRider);

	Interaction->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCRidingComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CheckNull(Interaction);

	const auto animal = Cast<ACAnimal_AI>(OtherActor);
	CheckTrue(!!animal);

	const auto interactor = Cast<ACCommonCharacter>(OtherActor);
	CheckNull(interactor);

	SetInteractableCharacter(interactor, nullptr);
	SetInteractableCharacter(Owner, nullptr);

	if (!!interactor->OnInteract.IsBound())
		interactor->OnInteract.RemoveDynamic(this, &UCRidingComponent::SetRider);

	Interaction->SetVisibility(ESlateVisibility::Collapsed);
}

void UCRidingComponent::SetInteractableCharacter(TWeakObjectPtr<ACCommonCharacter> InCharacter, 
	const TWeakObjectPtr<ACCommonCharacter> InOtherCharacter)
{
	InCharacter->SetInteractor(InOtherCharacter.Get());
}

void UCRidingComponent::SetRider(ACCommonCharacter* InCharacter)
{
	Rider = InCharacter;
	RiderWeaponComp = CHelpers::GetComponent<UCWeaponComponent>(InCharacter);
}

void UCRidingComponent::Tick_ToMountPoint()
{
	if (nullptr == RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)])
		CheckValidPoint();
	else
	{
		if (MoveToPoint(Rider, RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)]))
		{
			Rider->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
			Rider->GetCharacterMovement()->bEnablePhysicsInteraction = false;

			RidingState = ERidingState::Mounting;
		}
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

		MountRotationZFactor = 30.0f;
	}
	else if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateRight, 5.0f))
	{
		MountDir = EDirection::Right;
		RidingPoints[static_cast<uint8>(ERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(ERidingPoint::CandidateRight)];

		CHelpers::LoadAsset<UAnimMontage>(&MountAnim,
			TEXT("AnimMontage'/Game/Character/AnimationMontages/Riding/Rider_Mount_Front_Right_Montage.Rider_Mount_Front_Right_Montage'"));

		MountRotationZFactor = -30.0f;
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

void UCRidingComponent::Tick_Mounting()
{
	// 공통변수 세팅
	const TEnumAsByte<EMoveComponentAction::Type> eMoveAction = EMoveComponentAction::Type::Move;

	FLatentActionInfo latentInfo;
	latentInfo.CallbackTarget = this;


	// 탑승 후 위치, 방향으로
	FVector ridingPos = RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentLocation();
	FVector riderPos = Rider->GetActorLocation();

	const FVector dir = UKismetMathLibrary::GetDirectionUnitVector(ridingPos, riderPos);

	FVector targetLocation = ridingPos + dir * 50.0f;
	// targetLocation.Z += 70.0f;

	const FRotator rotator = UKismetMathLibrary::FindLookAtRotation(riderPos, ridingPos);

	const FRotator targetRotation = FRotator(0, 0, rotator.Yaw + MountRotationZFactor);

	UKismetSystemLibrary::MoveComponentTo(Rider->GetRootComponent(), targetLocation, targetRotation,
		false, true, 0.3f, false,
		eMoveAction, latentInfo);


	// 탑승중애니 - 탑승후루프애니 블렌딩
	Rider->PlayAnimMontage(MountAnim, 1.5f);
	
	const FRotator ridingRot = RidingPoints[static_cast<uint8>(ERidingPoint::Rider)]->GetComponentRotation();

	UKismetSystemLibrary::MoveComponentTo(Rider->GetRootComponent(), ridingPos, ridingRot,
		false, true, 0.2f, false,
		eMoveAction, latentInfo);


	// Attach - 몽타주 끝나는 시점에 딱
	if (false == Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.IsBound())
		Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(
			this, &UCRidingComponent::AttachToRiderPoint);


	UGameplayStatics::PlaySoundAtLocation(this, MountSound, Owner->GetActorLocation());

	RidingState = ERidingState::MountingEnd;
}

void UCRidingComponent::Tick_MountingEnd()
{
	// 애니 끝날 때까지 대기 후 실행
	if (false == IsValid(Rider->GetCurrentMontage()))
	{
		Rider->StateComp->SetRideMode();
		Rider->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		Rider->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		if (!!RiderWeaponComp->GetEquipment() &&
			!!RiderWeaponComp->GetEquipment()->GetEquipped())
			MovementComp->DisableControlRotation();
		else
			MovementComp->EnableControlRotation();

		MovementComp->Move();

		if (!!MovementComp->GetFixedCamera())
			MovementComp->FixCamera();
		else
			MovementComp->UnFixCamera();

		MovementComp->SetSpeed(ESpeedType::Sprint);

		// TODO Riding Info
		SetStatusUI();
		OnStatusUI();

		// AI Controller 세이브 / Player Controller 빙의
		ControllerSave = Owner->GetController();

		TWeakObjectPtr<ACCommonCharacter> animal = Cast<ACCommonCharacter>(Owner);

		GetWorld()->GetFirstPlayerController()->UnPossess();

		if (!!animal.Get())
			GetWorld()->GetFirstPlayerController()->Possess(animal.Get());

		Rider->SetMyCurController(GetWorld()->GetFirstPlayerController());


		RidingState = ERidingState::Riding;
	}
}

void UCRidingComponent::Tick_Riding()
{
	// TODO Zoom
	// TODO Height UI
}

void UCRidingComponent::Tick_Unmounting()
{
}

void UCRidingComponent::Tick_UnmountingEnd()
{
}

void UCRidingComponent::AttachToRiderPoint(UAnimMontage* Anim, bool bInterrupted)
{
	Rider->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, "RiderPoint");
}