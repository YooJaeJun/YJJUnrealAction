#include "Components/CMovementComponent.h"
#include "Components/CStateComponent.h"
#include "Components/CCamComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Utilities/CLog.h"

UCMovementComponent::UCMovementComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());

	if (Owner.IsValid())
		StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());

	const int32 speedSlotCountLocal = static_cast<int32>(CESpeedType::Max);
	Speeds.SetNum(speedSlotCountLocal);
	Speeds[static_cast<int32>(CESpeedType::Walk)] = 200.f;
	Speeds[static_cast<int32>(CESpeedType::Run)] = 500.f;
	Speeds[static_cast<int32>(CESpeedType::Sprint)] = 800.f;
}

void UCMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<ACCommonCharacter>(GetOwner());
	if (Owner.IsValid())
		StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());

	CachedOwnerCharacterActor = Cast<ACharacter>(GetOwner());
	if (false == IsValid(CachedOwnerCharacterActor.Get()))
	{
		CLog::Log(FString::Printf(TEXT("[MovementComponent] BeginPlay: Owner 가 Character 타입 아님 — Owner=%s"),
			IsValid(GetOwner()) ? *GetOwner()->GetName() : TEXT("nullptr")));
	}
}

void UCMovementComponent::OnEnableTopViewCam()
{
	bTopViewCam = true;
}

void UCMovementComponent::OnDisableTopViewCam()
{
	bTopViewCam = false;
}

void UCMovementComponent::SetSpeeds(const TArray<float>& InSpeeds)
{
	const int32 ceilingLocal = static_cast<int32>(CESpeedType::Max);
	if (Speeds.Num() < ceilingLocal)
		Speeds.SetNum(ceilingLocal);

	const int32 copyCountLocal = FMath::Min(InSpeeds.Num(), ceilingLocal);
	for (int32 copyIndexLocal = 0; copyIndexLocal < copyCountLocal; ++copyIndexLocal)
		Speeds[copyIndexLocal] = InSpeeds[copyIndexLocal];
}

void UCMovementComponent::SetSpeed(CESpeedType InType)
{
	if (false == Owner.IsValid())
	{
		CLog::Log(TEXT("[MovementComponent] SetSpeed: ACCommonCharacter Owner 없음"));
		return;
	}

	UCharacterMovementComponent* movementComponentLocal = Owner->GetCharacterMovement();
	if (false == IsValid(movementComponentLocal))
	{
		CLog::Log(FString::Printf(TEXT("[MovementComponent] SetSpeed: CharacterMovement 없음 — %s"),
			*Owner->GetName()));
		return;
	}

	const int32 speedIndexLocal = static_cast<int32>(InType);
	if (false == Speeds.IsValidIndex(speedIndexLocal))
	{
		CLog::Log(FString::Printf(TEXT("[MovementComponent] SetSpeed: Speeds 인덱스 없음 %d"),
			speedIndexLocal));
		return;
	}

	movementComponentLocal->MaxWalkSpeed = Speeds[speedIndexLocal];
}

void UCMovementComponent::SetSpeedFromLegacyUnderlyingByte(uint8 InUnderlyingValue)
{
	const uint8 maxExcludedLocal = static_cast<uint8>(CESpeedType::Max);
	uint8 clampedLocal = InUnderlyingValue;
	if (clampedLocal >= maxExcludedLocal)
	{
		clampedLocal = static_cast<uint8>(CESpeedType::Walk);
	}
	SetSpeed(static_cast<CESpeedType>(clampedLocal));
}

void UCMovementComponent::Move()
{
	bCanMove = true;
}

void UCMovementComponent::Stop()
{
	bCanMove = false;
}

float UCMovementComponent::GetWalkSpeed() const
{
	const int32 walkTierIndexLocal = static_cast<int32>(CESpeedType::Walk);
	return Speeds.IsValidIndex(walkTierIndexLocal) ? Speeds[walkTierIndexLocal] : 200.f;
}

float UCMovementComponent::GetRunSpeed() const
{
	const int32 runTierIndexLocal = static_cast<int32>(CESpeedType::Run);
	return Speeds.IsValidIndex(runTierIndexLocal) ? Speeds[runTierIndexLocal] : 500.f;
}

float UCMovementComponent::GetSprintSpeed() const
{
	const int32 sprintTierIndexLocal = static_cast<int32>(CESpeedType::Sprint);
	return Speeds.IsValidIndex(sprintTierIndexLocal) ? Speeds[sprintTierIndexLocal] : 800.f;
}

void UCMovementComponent::FixCamera()
{
	if (false == Owner.IsValid())
		return;

	UCCamComponent* camComponentLocal = YJJHelpers::GetComponent<UCCamComponent>(Owner.Get());
	if (IsValid(camComponentLocal))
		camComponentLocal->EnableFixedCamera();
}

void UCMovementComponent::UnFixCamera()
{
	if (false == Owner.IsValid())
		return;

	UCCamComponent* camComponentLocal = YJJHelpers::GetComponent<UCCamComponent>(Owner.Get());
	if (IsValid(camComponentLocal))
		camComponentLocal->DisableFixedCamera();
}

ACharacter* UCMovementComponent::ResolveOwningCharacterUnchecked() const
{
	if (IsValid(CachedOwnerCharacterActor.Get()))
		return CachedOwnerCharacterActor.Get();

	return Cast<ACharacter>(GetOwner());
}

void UCMovementComponent::EnableControlRotation()
{
	ACharacter* characterLocal = ResolveOwningCharacterUnchecked();
	// CachedOwnerCharacter 미갱신·비 캐릭터 소유 등으로 자주 발생할 수 있음 — 매 프레임 경로 아님, 로그 생략
	if (nullptr == characterLocal)
		return;

	characterLocal->bUseControllerRotationYaw = true;

	UCharacterMovementComponent* movementLocal = characterLocal->GetCharacterMovement();
	if (IsValid(movementLocal))
		movementLocal->bOrientRotationToMovement = false;
}

void UCMovementComponent::DisableControlRotation()
{
	ACharacter* characterLocal = ResolveOwningCharacterUnchecked();
	if (nullptr == characterLocal)
		return; // EnableControlRotation 과 동일 조건

	characterLocal->bUseControllerRotationYaw = false;

	UCharacterMovementComponent* movementLocal = characterLocal->GetCharacterMovement();
	if (IsValid(movementLocal))
		movementLocal->bOrientRotationToMovement = true;
}

void UCMovementComponent::SetMaxWalkSpeed(const float Speed) const
{
	Owner->GetCharacterMovement()->MaxWalkSpeed = Speed;
}

void UCMovementComponent::SetWalkSpeed() const
{
	const int32 walkTierLocal = static_cast<int32>(CESpeedType::Walk);
	if (Speeds.IsValidIndex(walkTierLocal))
		Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[walkTierLocal];
}

void UCMovementComponent::SetRunSpeed() const
{
	const int32 runTierLocal = static_cast<int32>(CESpeedType::Run);
	if (Speeds.IsValidIndex(runTierLocal))
		Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[runTierLocal];
}

void UCMovementComponent::SetSprintSpeed() const
{
	const int32 sprintTierLocal = static_cast<int32>(CESpeedType::Sprint);
	if (Speeds.IsValidIndex(sprintTierLocal))
		Owner->GetCharacterMovement()->MaxWalkSpeed = Speeds[sprintTierLocal];
}

void UCMovementComponent::SetGravity(float InGravity)
{
	ACharacter* characterResolved = ResolveOwningCharacterUnchecked();
	if (nullptr == characterResolved)
	{
		CLog::Log(TEXT("[MovementComponent] SetGravity: Character 미해결(Owner/Cached)."));
		return;
	}

	UCharacterMovementComponent* movementResolved = characterResolved->GetCharacterMovement();
	if (false == IsValid(movementResolved))
	{
		CLog::Log(FString::Printf(TEXT("[MovementComponent] SetGravity: CharacterMovement 없음 — %s"),
			*characterResolved->GetName()));
		return;
	}

	movementResolved->GravityScale = InGravity;
}

void UCMovementComponent::AddGravity(float InFactor)
{
	ACharacter* characterResolved = ResolveOwningCharacterUnchecked();
	if (nullptr == characterResolved)
	{
		CLog::Log(TEXT("[MovementComponent] AddGravity: Character 미해결(Owner/Cached)."));
		return;
	}

	UCharacterMovementComponent* movementResolved = characterResolved->GetCharacterMovement();
	if (false == IsValid(movementResolved))
	{
		CLog::Log(FString::Printf(TEXT("[MovementComponent] AddGravity: CharacterMovement 없음 — %s"),
			*characterResolved->GetName()));
		return;
	}

	movementResolved->GravityScale *= InFactor;
}

void UCMovementComponent::SetLerpMove(const bool bIn)
{
	bLerpMove = bIn;

	if (false == bIn)
		return;

	AActor* ownerActorResolved = GetOwner();
	if (false == IsValid(ownerActorResolved))
	{
		CLog::Log(TEXT("[MovementComponent] SetLerpMove(true): Owner 없음."));
		return;
	}

	const FVector anchorLocationResolved = ownerActorResolved->GetActorLocation();
	const FVector offsetForwardScaled =
		ownerActorResolved->GetActorForwardVector() * LerpMoveDistance;
	Dest = anchorLocationResolved + offsetForwardScaled;
}

void UCMovementComponent::SetJumpZ(const float InVelocity) const
{
	Owner->GetCharacterMovement()->JumpZVelocity = InVelocity;
}

void UCMovementComponent::SetFriction(const float InFriction, const float InBraking) const
{
	Owner->GetCharacterMovement()->GroundFriction = InFriction;
	Owner->GetCharacterMovement()->BrakingDecelerationWalking = InBraking;
}

void UCMovementComponent::InputAxis_MoveForward(const float InAxis)
{
	Forward = InAxis;

	CheckFalse(CanMove(InAxis));
	CheckTrue(StateComp->IsFallMode());

	const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
	FVector direction = FQuat(rotator).GetForwardVector();

	if (true == bTopViewCam)
		direction = FVector::XAxisVector;

	Owner->AddMovementInput(direction, InAxis);
}

void UCMovementComponent::InputAxis_MoveRight(const float InAxis)
{
	Right = InAxis;

	CheckFalse(CanMove(InAxis));
	CheckTrue(StateComp->IsFallMode());

	const FRotator rotator = FRotator(0, Owner->GetControlRotation().Yaw, 0);
	FVector direction = FQuat(rotator).GetRightVector();

	if (true == bTopViewCam)
		direction = FVector::YAxisVector;

	Owner->AddMovementInput(direction, InAxis);
}

void UCMovementComponent::InputAction_Walk()
{
	const int32 walkTierLocal = static_cast<int32>(CESpeedType::Walk);
	SetMaxWalkSpeed(Speeds.IsValidIndex(walkTierLocal) ? Speeds[walkTierLocal] : 200.f);

	SpeedFactor = 1.0f;
}

void UCMovementComponent::InputAction_Run()
{
	const int32 sprintTierLocal = static_cast<int32>(CESpeedType::Sprint);
	SetMaxWalkSpeed(Speeds.IsValidIndex(sprintTierLocal) ? Speeds[sprintTierLocal] : 800.f);

	SpeedFactor = 4.0f;
}

void UCMovementComponent::InputAction_Jump()
{
	CheckFalse(CanMove());

	Owner->Jump();
	StateComp->SetFallMode();
}

bool UCMovementComponent::CanMove(const float InAxis) const
{
	if (FMath::Abs(InAxis) > 0.5f)
		return CanMove();

	return false;
}

bool UCMovementComponent::IsCanMove() const
{
	return CanMove();
}

bool UCMovementComponent::GetFixedCamera() const
{
	if (false == Owner.IsValid())
		return false;

	const TObjectPtr<UCCamComponent> camComp = YJJHelpers::GetComponent<UCCamComponent>(Owner.Get());
	if (false == IsValid(camComp))
		return false;

	return camComp->GetFixedCamera();
}
