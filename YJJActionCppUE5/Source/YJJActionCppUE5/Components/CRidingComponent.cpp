#include "Components/CRidingComponent.h"
#include "Utilities/YJJLocalizedText.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Characters/Animals/CAnimal.h"
#include "Commons/CGameMode.h"
#include "Commons/CPlayerController.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"
#include "Blueprint/UserWidget.h"
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
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CWeaponComponent.h"
#include "Weapons/CEquipment.h"
#include "GameFramework/Controller.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Weapons/CAct.h"
#include "Components/CGameUIComponent.h"
#include "Components/CTargetingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UnrealType.h"
#include "Kismet/KismetMathLibrary.h"

namespace
{
void FindMountSceneComponent(ACAnimal* Animal, TObjectPtr<USceneComponent>& OutMount)
{
	OutMount = nullptr;
	if (!IsValid(Animal))
		return;

	TArray<UActorComponent*> comps;
	Animal->GetComponents(comps);
	for (int32 i = 0; i < comps.Num(); ++i)
	{
		UActorComponent* comp = comps[i];
		if (!IsValid(comp))
			continue;
		if (comp->GetFName() != FName(TEXT("Mount")))
			continue;

		USceneComponent* scene = Cast<USceneComponent>(comp);
		if (IsValid(scene))
			OutMount = scene;
		return;
	}
}

TObjectPtr<UActorComponent> FindRiderMagicComponent(ACCommonCharacter* InRider)
{
	if (!IsValid(InRider))
		return nullptr;

	TArray<UActorComponent*> comps;
	InRider->GetComponents(comps);
	for (int32 i = 0; i < comps.Num(); ++i)
	{
		UActorComponent* comp = comps[i];
		if (!IsValid(comp))
			continue;

		const FString className = comp->GetClass()->GetName();
		if (className.Contains(TEXT("Magic")) && className.Contains(TEXT("Component")))
			return comp;
	}
	return nullptr;
}

UUserWidget* FindNestedUserWidgetByPropertyName(UUserWidget* Root, const FName PropertyName)
{
	if (!IsValid(Root))
		return nullptr;

	for (TFieldIterator<FObjectProperty> It(Root->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->GetFName() != PropertyName)
			continue;

		UObject* valueObject = It->GetObjectPropertyValue_InContainer(Root);
		return Cast<UUserWidget>(valueObject);
	}
	return nullptr;
}
}

UCRidingComponent::UCRidingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACAnimal>(GetOwner());

	eMoveAction = EMoveComponentAction::Type::Move;
	latentInfo.CallbackTarget = this;

	YJJHelpers::GetAsset<UTexture2D>(&InteractionKeyTexture,
		TEXT("Texture2D'/Game/Assets/Textures/ButtonPrompts/F_Key_Dark.F_Key_Dark'"));

	YJJHelpers::GetAsset<UAnimMontage>(&MountAnimByDirection[static_cast<uint8>(CEDirection::Left)],
		TEXT("AnimMontage'/Game/Character/Player/Montages/Riding/Rider_Mount_Front_Left_Montage.Rider_Mount_Front_Left_Montage'"));

	YJJHelpers::GetAsset<UAnimMontage>(&MountAnimByDirection[static_cast<uint8>(CEDirection::Right)],
		TEXT("AnimMontage'/Game/Character/Player/Montages/Riding/Rider_Mount_Front_Right_Montage.Rider_Mount_Front_Right_Montage'"));

	YJJHelpers::GetAsset<UAnimMontage>(&MountAnimByDirection[static_cast<uint8>(CEDirection::Back)],
		TEXT("AnimMontage'/Game/Character/Player/Montages/Riding/Rider_Mount_Back_Montage.Rider_Mount_Back_Montage'"));

	YJJHelpers::GetAsset<USoundBase>(&MountSound,
		TEXT("SoundWave'/Game/Assets/Sounds/Action/Sway_2.Sway_2'"));

	YJJHelpers::GetAsset<UAnimMontage>(&UnmountAnim,
		TEXT("AnimMontage'/Game/Character/Player/Montages/Riding/Rider_Dismount_Front_Right_Montage.Rider_Dismount_Front_Right_Montage'"));

	YJJHelpers::GetAsset<USoundBase>(&UnmountSound,
		TEXT("SoundWave'/Game/Assets/Sounds/Action/Sway_2.Sway_2'"));
}

TWeakObjectPtr<ACCommonCharacter> UCRidingComponent::GetRider() const
{
	return Rider;
}

void UCRidingComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACAnimal>(GetOwner());
	CheckNull(Owner.Get());

	// BP RidingComponent — Begin Play / UI Event: 오너(탈것)의 컴포넌트·포인트 캐시. (Eye 스폰·RiderPoint 부착은 ACAnimal::BeginPlay 에서 처리.)
	Mesh = Owner->GetMesh();
	SpringArm = Owner->GetSpringArm();
	Camera = Owner->GetCamera();
	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
	MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());
	CamComp = YJJHelpers::GetComponent<UCCamComponent>(Owner.Get());

	StateComponent = StateComp.Get();
	Moving = MovementComp.Get();

	MountLeft = Owner->GetMountLeftPoint();
	MountRight = Owner->GetMountRightPoint();
	MountBack = Owner->GetMountBackPoint();
	RiderPoint = Owner->GetRiderPoint();
	UnmountPoint = Owner->GetUnmountPoint();

	FindMountSceneComponent(Owner.Get(), Mount);

	RidingPoints[static_cast<uint8>(CERidingPoint::CandidateLeft)] = MountLeft;
	RidingPoints[static_cast<uint8>(CERidingPoint::CandidateRight)] = MountRight;
	RidingPoints[static_cast<uint8>(CERidingPoint::CandidateBack)] = MountBack;
	RidingPoints[static_cast<uint8>(CERidingPoint::Rider)] = RiderPoint;
	RidingPoints[static_cast<uint8>(CERidingPoint::Unmount)] = UnmountPoint;

	InteractionCollision = Owner->GetInteractionCollision();

	ApplyMountAnimsFromBlueprintArray();

	EyeClass = Owner->GetEyeActorClass();
	Eye = Owner->GetSpawnedEyeActor();

	if (CamComp.IsValid())
	{
		ZoomData = CamComp->ZoomData;
		// 블루프린트 MovingComponent::DisableControlRotation / Cam 정리에 대응.
		CamComp->DisableControlRotation();
	}

	if (IsValid(SpringArm.Get()))
	{
		// 블루프린트 SetZooming(InZooming <- SpringArm.TargetArmLength) — 입력 줌 베이스값.
		Zooming = static_cast<double>(SpringArm->TargetArmLength);
		if (CamComp.IsValid())
			CamComp->SetZooming(static_cast<float>(Zooming));
	}

	BindRidingAnimalUiDelegates();
	RefreshRidingAnimalHudFromAnimal();

	if (UKismetSystemLibrary::IsDedicatedServer(this))
		return;

	const TWeakObjectPtr<ACPlayerController> playerController = Cast<ACPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (playerController.IsValid())
		Hud = playerController->EnsureHUD();
	else
		CLog::Log(FString::Printf(TEXT("[UI] UCRidingComponent::BeginPlay: 로컬 PlayerController(AC) 없음 — Owner=%s"), *GetNameSafe(Owner.Get())));

	if (false == Hud.IsValid())
	{
		CLog::Log(FString::Printf(TEXT("[UI] UCRidingComponent::BeginPlay: HUD 없음 — Owner=%s"), *GetNameSafe(Owner.Get())));
		return;
	}

	Hud->SetChildren();
	Interaction = Hud->Interaction;

	InteractionText = YJJLocalization::LocalizedText_MountInteract();

	CheckNull(Interaction.Get());
	Interaction->SetChildren(InteractionKeyTexture, InteractionText);
}

void UCRidingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindRidingAnimalUiDelegates();
	Super::EndPlay(EndPlayReason);
}

void UCRidingComponent::ApplyMountAnimsFromBlueprintArray()
{
	// BPVar MountAnims TArray — Left, Right, Back 순(3개)이면 C++ 방향별 슬롯을 덮어쓴다.
	if (MountAnims.Num() < 3)
		return;

	MountAnimByDirection[static_cast<uint8>(CEDirection::Left)] = MountAnims[0];
	MountAnimByDirection[static_cast<uint8>(CEDirection::Right)] = MountAnims[1];
	MountAnimByDirection[static_cast<uint8>(CEDirection::Back)] = MountAnims[2];
}

void UCRidingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (RidingState)
	{
	case CERidingState::None:
		if (IsValid(Rider.Get()))
			SetRidingState(CERidingState::MovingToMountPoint);
		break;
	case CERidingState::MovingToMountPoint:
		Tick_MovingToMountPoint();
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
	case CERidingState::RidingEnd:
		Tick_RidingEnd();
		break;
	}
}

void UCRidingComponent::SetRidingState(const CERidingState InState)
{
	RidingState = InState;
}

void UCRidingComponent::MountInteraction_OnBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	CheckNull(Interaction.Get());
	CheckNull(Owner.Get());

	InteractableActor = OtherActor;

	const TWeakObjectPtr<ACPlayableCharacter> interactor = Cast<ACPlayableCharacter>(OtherActor);
	CheckNull(interactor.Get());

	const TWeakObjectPtr<ACCommonCharacter> mountAsCommon = TWeakObjectPtr<ACCommonCharacter>(Owner.Get());
	SetInteractor(interactor, mountAsCommon);
	SetInteractor(mountAsCommon, interactor);

	if (false == interactor->OnMount.IsBound())
		interactor->OnMount.AddUniqueDynamic(this, &UCRidingComponent::SetRider);

	Interaction->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCRidingComponent::MountInteraction_OnEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	CheckNull(Interaction.Get());
	CheckNull(Owner.Get());

	InteractableActor = nullptr;

	const TWeakObjectPtr<ACPlayableCharacter> interactor = Cast<ACPlayableCharacter>(OtherActor);
	CheckNull(interactor.Get());

	const TWeakObjectPtr<ACCommonCharacter> mountAsCommon = TWeakObjectPtr<ACCommonCharacter>(Owner.Get());
	SetInteractor(interactor, nullptr);
	SetInteractor(mountAsCommon, nullptr);

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
	RiderWeapon = RiderWeaponComp.Get();
	RiderMagic = FindRiderMagicComponent(InCharacter);

	if (false == Owner->OnUnmount.IsBound())
		Owner->OnUnmount.AddDynamic(this, &UCRidingComponent::Unmount);
}

void UCRidingComponent::Tick_MovingToMountPoint()
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

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateLeft, 50))
	{
		RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(CERidingPoint::CandidateLeft)];

		MountDir = CEDirection::Left;

		MountRotationZFactor = 100.0;
	}
	else if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateRight, 50))
	{
		RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(CERidingPoint::CandidateRight)];

		MountDir = CEDirection::Right;

		MountRotationZFactor = -100.0;
	}
	else if (UKismetMathLibrary::NearlyEqual_FloatFloat(minCandidate, candidateBack, 50))
	{
		RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)] =
			RidingPoints[static_cast<uint8>(CERidingPoint::CandidateBack)];

		MountDir = CEDirection::Back;

		MountRotationZFactor = 0.0;
	}

	MountAnim = MountAnimByDirection[static_cast<uint8>(MountDir)];
}

bool UCRidingComponent::MoveToPoint(
	TWeakObjectPtr<ACCommonCharacter> Char, 
	const TWeakObjectPtr<USceneComponent> To)
{
	bool reached = true;

	if (UKismetMathLibrary::NearlyEqual_FloatFloat(Char->GetActorLocation().X, To->GetComponentLocation().X, 15.0f) &&
		UKismetMathLibrary::NearlyEqual_FloatFloat(Char->GetActorLocation().Y, To->GetComponentLocation().Y, 15.0f))
	{
		Rider->GetCharacterMovement()->StopMovementImmediately();
		RidingPoints[static_cast<uint8>(CERidingPoint::CurMount)] = nullptr;
		reached = true;
	}
	else
	{
		const FVector unitDirection = 
			UKismetMathLibrary::GetDirectionUnitVector(Char->GetActorLocation(), To->GetComponentLocation());
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
	targetPos.Z += 20.0f;

	FRotator targetRot = RidingPoints[static_cast<uint8>(CERidingPoint::Rider)]->GetComponentRotation();
	targetRot = FRotator(0, targetRot.Yaw + MountRotationZFactor, 0);

	latentInfo.CallbackTarget = Rider.Get();

	// 탑승 후 위치, 방향으로
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

	Rider->PlayAnimMontage(MountAnimByDirection[static_cast<uint8>(MountDir)], 1.0f);

	CheckNull(Rider->GetMesh());
	CheckNull(Rider->GetMesh()->GetAnimInstance());

	// 탑승중애니 - 탑승후루프애니
	// 몽타주 블렌드 아웃 되는 시점에 딱
	if (false == Rider->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.IsBound())
		Rider->GetMesh()->GetAnimInstance()->OnMontageBlendingOut.AddUniqueDynamic(
			this, &UCRidingComponent::InterpToRiderPos);

	// Attach - 몽타주 끝나는 시점에 딱
	if (false == Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.IsBound())
		Rider->GetMesh()->GetAnimInstance()->OnMontageEnded.AddUniqueDynamic(
			this, &UCRidingComponent::AttachToRiderPoint);


	UGameplayStatics::PlaySoundAtLocation(this, MountSound, Owner->GetActorLocation());

	SetRidingState(CERidingState::MountingEnd);
}

void UCRidingComponent::InterpToRiderPos(UAnimMontage* Anim, bool bInterrupted)
{
	const FVector riderPos = RidingPoints[static_cast<uint8>(CERidingPoint::Rider)]->GetComponentLocation();
	const FRotator riderRot = RidingPoints[static_cast<uint8>(CERidingPoint::Rider)]->GetComponentRotation();

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

	SpringArm->bUsePawnControlRotation = true;

	Rider->SetLegIKAlpha(static_cast<float>(LegIKAlpha));


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

	// TODO Zoom
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
		Rider->GetActorLocation().Z + 30.0f));

	Rider->SetActorRotation(FRotator(0, 0, 0));

	Rider->PlayAnimMontage(UnmountAnim, 1.2f);


	UnpossessAndInterpToCamera();

	// 탑승 후 위치, 방향으로
	FVector targetPos = RidingPoints[static_cast<uint8>(CERidingPoint::Unmount)]->GetComponentLocation();
	targetPos.Z += 20.0f;
	FRotator targetRot = RidingPoints[static_cast<uint8>(CERidingPoint::Unmount)]->GetComponentRotation();
	targetRot = FRotator(0, targetRot.Yaw + MountRotationZFactor, 0);

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

	SetRidingState(CERidingState::RidingEnd);
}

void UCRidingComponent::UnpossessAndInterpToCamera()
{
	// Camera Start
	Camera->SetWorldTransform(UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetTransform());

	// Controller Possess
	TWeakObjectPtr<ACCommonCharacter> animal = Cast<ACCommonCharacter>(Owner);

	GetWorld()->GetFirstPlayerController()->UnPossess();

	if (IsValid(Rider.Get()))
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

void UCRidingComponent::Tick_RidingEnd()
{
	// 애니 끝날 때까지 대기 후 실행
	if (false == IsValid(Rider->GetCurrentMontage()))
	{
		SetInteractor(TWeakObjectPtr<ACCommonCharacter>(Owner.Get()), TWeakObjectPtr<ACCommonCharacter>(Rider.Get()));
		SetInteractor(TWeakObjectPtr<ACCommonCharacter>(Rider.Get()), TWeakObjectPtr<ACCommonCharacter>(Owner.Get()));

		RiderMovementComp->Move();

		Rider = nullptr;
		RiderWeapon = nullptr;
		RiderMagic = nullptr;
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

void UCRidingComponent::Ride_Input_Action()
{
	InputAction_Act();
}

void UCRidingComponent::Ride_Input_SubWeaponPressed()
{
	if (false == RiderWeaponComp.IsValid())
		return;

	RiderWeaponComp->InputAction_SubAct_Pressed();
}

void UCRidingComponent::Ride_Input_SubWeaponReleased()
{
	if (false == RiderWeaponComp.IsValid())
		return;

	RiderWeaponComp->InputAction_SubAct_Released();
}

void UCRidingComponent::Ride_Input_Skill(const int32 InIndex)
{
	if (false == RiderWeaponComp.IsValid())
		return;

	if (InIndex == 0)
		RiderWeaponComp->InputAction_Skill_1_Pressed();
	else if (InIndex == 1)
		RiderWeaponComp->InputAction_Skill_2_Pressed();
}

void UCRidingComponent::Ride_Input_Skill1Pressed()
{
	Ride_Input_Skill(0);
}

void UCRidingComponent::Ride_Input_Skill2Pressed()
{
	Ride_Input_Skill(1);
}

void UCRidingComponent::CancelHitAnim()
{
	ACPlayableCharacter* riderPc = Cast<ACPlayableCharacter>(Rider.Get());
	if (IsValid(riderPc))
		riderPc->CancelHitAnim();
}

void UCRidingComponent::Skill_Unmount()
{
	// BP 레거시 훅. 필요 시 무기/스킬 탑승 전용 처리 추가.
}

void UCRidingComponent::Ride_Input_Magic()
{
	if (false == RiderWeaponComp.IsValid())
		return;

	// BP 의 Magic 입력 — C++ 플레이어는 Skill_3 슬롯을 쓰므로 탑승 중에도 동일하게 맞춘다.
	RiderWeaponComp->InputAction_Skill_3_Pressed();
}

void UCRidingComponent::ZoomInput(const double InAxis)
{
	if (false == CamComp.IsValid())
		return;

	CamComp->InputAxis_Zoom(static_cast<float>(InAxis));
}

void UCRidingComponent::SetZooming(const double InZooming)
{
	Zooming = InZooming;
	if (CamComp.IsValid())
	{
		CamComp->ZoomData = ZoomData;
		CamComp->SetZooming(static_cast<float>(InZooming));
	}
}

void UCRidingComponent::ApplyZoom(const double InZoom)
{
	if (false == IsValid(SpringArm.Get()))
		return;

	const float current = SpringArm->TargetArmLength;
	if (UKismetMathLibrary::NearlyEqual_FloatFloat(static_cast<double>(current), InZoom, 0.1))
		return;

	const float deltaSeconds = UGameplayStatics::GetWorldDeltaSeconds(this);
	const float nextLength = UKismetMathLibrary::FInterpTo(
		current,
		static_cast<float>(InZoom),
		deltaSeconds,
		ZoomData.InterpSpeed);

	SpringArm->TargetArmLength = nextLength;
	// CCamComponent::TickComponent 가 Zooming 을 목표로 또 보간하므로, 여기서 목표를 현재 길이에 맞춰 충돌을 막는다.
	Zooming = static_cast<double>(nextLength);
	if (CamComp.IsValid())
		CamComp->SetZooming(nextLength);
}

bool UCRidingComponent::IsBowMode() const
{
	if (false == RiderWeaponComp.IsValid())
		return false;

	return RiderWeaponComp->IsBowMode();
}

void UCRidingComponent::TargetingInput()
{
	if (false == IsValid(Owner.Get()))
		return;

	const TObjectPtr<UCTargetingComponent> targeting = Owner->GetTargetingComp();
	if (IsValid(targeting))
		targeting->InputAction_Targeting();
}

void UCRidingComponent::Ride_Input_Menu()
{
	if (false == IsValid(Owner.Get()))
		return;

	const TObjectPtr<UCGameUIComponent> gameUi = YJJHelpers::GetComponent<UCGameUIComponent>(Owner.Get());
	if (IsValid(gameUi))
		gameUi->InputAction_ActivateEquipMenu();
}

void UCRidingComponent::Ride_Input_MenuHide()
{
	if (false == IsValid(Owner.Get()))
		return;

	const TObjectPtr<UCGameUIComponent> gameUi = YJJHelpers::GetComponent<UCGameUIComponent>(Owner.Get());
	if (IsValid(gameUi))
		gameUi->InputAction_DeactivateEquipMenu();
}

void UCRidingComponent::Ride_Input_MagicMenu()
{
	if (false == IsValid(Rider.Get()))
		return;

	Rider->OnMagicMenu();
}

void UCRidingComponent::Ride_Input_MagicMenuHide()
{
	if (false == IsValid(Rider.Get()))
		return;

	Rider->OffMagicMenu();
}

void UCRidingComponent::Ride_Ui_HoveredEquip(const FString& InName)
{
	if (false == IsValid(Rider.Get()))
		return;

	Rider->HoveredEquipMenu(InName);
}

void UCRidingComponent::Ride_Ui_UnhoveredEquip(const FString& InName)
{
	if (false == IsValid(Rider.Get()))
		return;

	Rider->UnHoveredEquipMenu(InName);
}

void UCRidingComponent::Ride_Ui_ClickedEquip(const FString& InName)
{
	if (false == IsValid(Rider.Get()))
		return;

	Rider->ClickedEquipMenu(InName);
}

void UCRidingComponent::Ride_Ui_HoveredMagic(const FString& InName)
{
	if (false == IsValid(Rider.Get()))
		return;

	Rider->HoveredMagicMenu(InName);
}

void UCRidingComponent::Ride_Ui_UnhoveredMagic(const FString& InName)
{
	if (false == IsValid(Rider.Get()))
		return;

	Rider->UnHoveredMagicMenu(InName);
}

void UCRidingComponent::Ride_Ui_ClickedMagic(const FString& InName)
{
	if (false == IsValid(Rider.Get()))
		return;

	Rider->ClickedMagicMenu(InName);
}

void UCRidingComponent::GetAction(FText& OutText) const
{
	OutText = InteractionText;
}

void UCRidingComponent::EndInteraction()
{
	// BP: Interact → I_Interactable.EndInteraction — 탑승 중에만 하차 시퀀스.
	if (false == IsValid(Rider.Get()))
		return;
	if (RidingState != CERidingState::Riding)
		return;

	Unmount();
}

UCUserWidget_HUD* UCRidingComponent::ResolveLocalHud()
{
	if (Hud.IsValid())
		return Hud.Get();

	UWorld* world = GetWorld();
	if (!IsValid(world))
		return nullptr;

	APlayerController* pc = UGameplayStatics::GetPlayerController(world, 0);
	if (!IsValid(pc))
		return nullptr;

	ACPlayerController* yjjPc = Cast<ACPlayerController>(pc);
	if (IsValid(yjjPc))
		return yjjPc->EnsureHUD();

	return nullptr;
}

void UCRidingComponent::SetStatusUI()
{
	if (UKismetSystemLibrary::IsDedicatedServer(this))
		return;

	if (!IsValid(Owner.Get()))
		return;

	UCUserWidget_HUD* hudWidget = ResolveLocalHud();
	if (!IsValid(hudWidget))
		return;

	// 레거시 Blueprint — WB_HUDUI / CWB_HUD 가 UPROPERTY 로 노출한 변수명과 동일해야 한다.
	UUserWidget* ridingInfo = FindNestedUserWidgetByPropertyName(hudWidget, FName(TEXT("WB_RidingAnimalInfo")));
	if (IsValid(ridingInfo))
	{
		UUserWidget* hpBarW = FindNestedUserWidgetByPropertyName(ridingInfo, FName(TEXT("WB_HpBar_RidingAnimal")));
		if (IsValid(hpBarW))
			HpBar = hpBarW;

		UUserWidget* levelBarW = FindNestedUserWidgetByPropertyName(ridingInfo, FName(TEXT("WB_Level_RidingAnimal")));
		if (IsValid(levelBarW))
			LevelBar = levelBarW;

		// UDS FCharacterInfo Break 대신 네이티브 CharacterInfo.Name 사용.
		UFunction* setNameFn = ridingInfo->FindFunction(FName(TEXT("SetName")));
		if (IsValid(setNameFn))
		{
			struct FRidingAnimalSetNameParams
			{
				FString Name;
			};

			FRidingAnimalSetNameParams params;
			params.Name = Owner->CharacterInfo.Name.ToString();
			ridingInfo->ProcessEvent(setNameFn, &params);
		}
	}

	if (FlyingType)
	{
		UUserWidget* heightW = FindNestedUserWidgetByPropertyName(hudWidget, FName(TEXT("WB_Height")));
		if (IsValid(heightW))
			HeightUI = heightW;
	}
	else
		HeightUI = nullptr;

	RefreshRidingAnimalHudFromAnimal();
}

void UCRidingComponent::OnStatusUI(const bool InOn)
{
	if (UKismetSystemLibrary::IsDedicatedServer(this))
		return;

	UCUserWidget_HUD* hudWidget = ResolveLocalHud();
	if (!IsValid(hudWidget))
		return;

	UUserWidget* ridingInfo = FindNestedUserWidgetByPropertyName(hudWidget, FName(TEXT("WB_RidingAnimalInfo")));
	if (IsValid(ridingInfo))
		ridingInfo->SetVisibility(InOn ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (InOn)
	{
		if (FlyingType && IsValid(HeightUI))
			HeightUI->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		if (IsValid(HeightUI))
			HeightUI->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCRidingComponent::BindRidingAnimalUiDelegates()
{
	if (!IsValid(Owner.Get()))
		return;

	ACAnimal* animal = Owner.Get();
	CheckNull(animal);

	animal->OnUpdateHp_Riding.AddUniqueDynamic(this, &UCRidingComponent::OnRidingAnimalHpUiRequested);
	animal->OnUpdateExp_Riding.AddUniqueDynamic(this, &UCRidingComponent::OnRidingAnimalExpUiRequested);
	animal->OnUpdateLevel_Riding.AddUniqueDynamic(this, &UCRidingComponent::OnRidingAnimalLevelUiRequested);
}

void UCRidingComponent::UnbindRidingAnimalUiDelegates()
{
	if (!IsValid(Owner.Get()))
		return;

	ACAnimal* animal = Owner.Get();
	if (!IsValid(animal))
		return;

	animal->OnUpdateHp_Riding.RemoveAll(this);
	animal->OnUpdateExp_Riding.RemoveAll(this);
	animal->OnUpdateLevel_Riding.RemoveAll(this);
}

void UCRidingComponent::RefreshRidingAnimalHudFromAnimal()
{
	RefreshRidingHpBarWidgetFromAnimal();
	RefreshRidingLevelBarWidgetFromAnimal();
}

void UCRidingComponent::RefreshRidingHpBarWidgetFromAnimal()
{
	if (!IsValid(Owner.Get()))
		return;

	if (!IsValid(HpBar))
		return;

	UFunction* const setHpUi = HpBar->FindFunction(FName(TEXT("SetHPUI")));
	if (!IsValid(setHpUi))
		return;

	struct FSetHpUiParams
	{
		double InCur;
		double InMax;
	};

	FSetHpUiParams params;
	params.InCur = Owner->Hp;
	params.InMax = Owner->MaxHp;
	HpBar->ProcessEvent(setHpUi, &params);
}

void UCRidingComponent::RefreshRidingLevelBarWidgetFromAnimal()
{
	if (!IsValid(Owner.Get()))
		return;

	if (!IsValid(LevelBar))
		return;

	UFunction* const setLevelUi = LevelBar->FindFunction(FName(TEXT("SetLevelUI")));
	if (!IsValid(setLevelUi))
		return;

	struct FSetLevelUiParams
	{
		int32 InLevel;
		double InCurExp;
		double InMaxExp;
	};

	FSetLevelUiParams params;
	params.InLevel = Owner->Level;
	params.InCurExp = Owner->Exp;
	params.InMaxExp = Owner->MaxExp;
	LevelBar->ProcessEvent(setLevelUi, &params);
}

void UCRidingComponent::TryPlayRidingLevelBarLevelUpEffect()
{
	// BP: GameMode 를 프로젝트 GameMode 로 캐스트한 뒤에만 LevelBar.LevelUp 호출(캐스트 실패 시 연출 생략).
	UWorld* world = GetWorld();
	if (!IsValid(world))
		return;

	if (!IsValid(LevelBar))
		return;

	AGameModeBase* gmBase = UGameplayStatics::GetGameMode(world);
	if (!IsValid(gmBase))
		return;

	if (!Cast<ACGameMode>(gmBase))
		return;

	UFunction* const levelUpFn = LevelBar->FindFunction(FName(TEXT("LevelUp")));
	if (!IsValid(levelUpFn))
		return;

	LevelBar->ProcessEvent(levelUpFn, nullptr);
}

void UCRidingComponent::OnRidingAnimalHpUiRequested()
{
	RefreshRidingHpBarWidgetFromAnimal();
}

void UCRidingComponent::OnRidingAnimalExpUiRequested()
{
	RefreshRidingLevelBarWidgetFromAnimal();
}

void UCRidingComponent::OnRidingAnimalLevelUiRequested()
{
	TryPlayRidingLevelBarLevelUpEffect();
}