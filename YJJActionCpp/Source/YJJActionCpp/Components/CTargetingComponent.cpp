#include "Components/CTargetingComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "CStateComponent.h"
#include "CMovementComponent.h"
#include "CCamComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Controller.h"

UCTargetingComponent::UCTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACCommonCharacter>(GetOwner());

	if (Owner.IsValid())
		Controller = Cast<AController>(Owner->GetMyCurController());

	MovingFocus_CurrentCoolTime = MovingFocus_ConstantTime;
}

void UCTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Tick_MoveFocusCoolTIme(DeltaTime);
	Tick_Targeting();
}

void UCTargetingComponent::InputAction_Targeting()
{
	if (bTargeting)
		End_Targeting();
	else
		Begin_Targeting();
}

void UCTargetingComponent::Begin_Targeting()
{
	const TArray<AActor*> ignores { Owner.Get() };
	TArray<FHitResult> hitResults;

	UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(),
		Owner->GetActorLocation(), 
		Owner->GetActorLocation(),
		TraceDistance, 
		"Targeting",
		false, 
		ignores,
		EDrawDebugTrace::None,
		hitResults, 
		true);

	TArray<TWeakObjectPtr<ACCommonCharacter>> targets;

	for (const auto& elem : hitResults)
	{
		if (elem.GetActor()->GetClass() != Owner->GetClass())
		{
			targets.Push(Cast<ACCommonCharacter>(elem.GetActor()));
			bTargeting = true;
		}
	}

	const TWeakObjectPtr<ACCommonCharacter> target = 
		YJJHelpers::GetNearForward(Owner, targets, Owner->GetMyCurController());

	ChangeTarget(target.Get());
}

void UCTargetingComponent::End_Targeting()
{
	SetVisibleTargetUI(false);

	CheckNull(Target);

	const TWeakObjectPtr<UCStateComponent> targetState =
		YJJHelpers::GetComponent<UCStateComponent>(Target.Get());
	CheckNull(targetState);

	const TWeakObjectPtr<UCCamComponent> targetCamComp =
		YJJHelpers::GetComponent<UCCamComponent>(Target.Get());
	CheckNull(targetCamComp);

	targetCamComp->DisableFixedCamera();
	Target = nullptr;
	TargetStateComp = nullptr;
	TargetMovementComp = nullptr;
	TargetingWidgetComp = nullptr;
	bTargeting = false;
}

void UCTargetingComponent::ChangeTarget(ACCommonCharacter* InTarget)
{
	if (IsValid(InTarget))
	{
		if (Target.IsValid())
			SetVisibleTargetUI(false);

		Target = InTarget;

		if (Target.IsValid())
		{
			TargetStateComp = YJJHelpers::GetComponent<UCStateComponent>(Target.Get());
			TargetMovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Target.Get());
			TargetingWidgetComp = YJJHelpers::GetComponent<UWidgetComponent>(Target.Get());
			SetVisibleTargetUI(true);

			if (TargetMovementComp.IsValid())
				TargetCamComp->EnableFixedCamera();
		}
	}
	else
	{
		bTargeting = false;
		End_Targeting();
	}
}

void UCTargetingComponent::SetVisibleTargetUI(bool bVisible) const
{
	CheckNull(Target);
	CheckNull(TargetingWidgetComp);

	TargetingWidgetComp->SetVisibility(bVisible);
}

void UCTargetingComponent::Tick_MoveFocusCoolTIme(const float InDelta)
{
	CheckTrue(bCanMoveFocus);

	MovingFocus_CurrentCoolTime -= InDelta;

	if (MovingFocus_CurrentCoolTime < 0.0f)
	{
		MovingFocus_CurrentCoolTime = MovingFocus_ConstantTime;
		bCanMoveFocus = true;
	}
}

void UCTargetingComponent::Tick_Targeting()
{
	CheckNull(Target);
	CheckNull(TargetStateComp);

	bool bIsValid = true;
	bIsValid &= (false == TargetStateComp->IsDeadMode());
	bIsValid &= (true == (Owner->GetDistanceTo(Target.Get()) <= TraceDistance));

	if (false == bIsValid)
	{
		End_Targeting();
		return;
	}

	if (nullptr == Controller.Get())
		Controller = Owner->GetMyCurController();

	const FRotator controlRotation = Controller->GetControlRotation();
	const FRotator ownerToTarget = UKismetMathLibrary::FindLookAtRotation(
		Owner->GetActorLocation(), Target->GetActorLocation());

	if (true == UKismetMathLibrary::EqualEqual_RotatorRotator(controlRotation, ownerToTarget, FinishAngle))
	{
		Controller->SetControlRotation(ownerToTarget);

		if (true == bMovingFocus)
			bMovingFocus = false;
	}
	else
	{
		const FRotator rotator = UKismetMathLibrary::RInterpTo(controlRotation, 
			ownerToTarget, 
			UGameplayStatics::GetWorldDeltaSeconds(GetWorld()), 
			InterpSpeed);
		
		Controller->SetControlRotation(rotator);
	}
}

void UCTargetingComponent::ChangeFocus(const bool InbRight)
{
	CheckFalse(bCanMoveFocus);
	CheckNull(Target);

	const TArray<AActor*> ignores{ Owner.Get(), Target.Get() };
	TArray<FHitResult> hitResults;

	UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(),
		Owner->GetActorLocation(),
		Owner->GetActorLocation(),
		TraceDistance,
		"Targeting",
		false,
		ignores,
		EDrawDebugTrace::None,
		hitResults,
		true);

	TArray<TWeakObjectPtr<ACCommonCharacter>> targets;

	CheckTrue(hitResults.Num() == 0);

	for (const auto& elem : hitResults)
	{
		bool bCheck = true;
		bCheck &= IsValid(elem.GetActor());
		bCheck &= (elem.GetActor()->GetClass() != Owner->GetClass());

		if (bCheck)
		{
			targets.AddUnique(Cast<ACCommonCharacter>(elem.GetActor()));
			bTargeting = true;
		}
	}

	TMap<float, TWeakObjectPtr<ACCommonCharacter>> nearCharacters;

	YJJHelpers::AddNearSideCharacters(Owner, targets, Owner->GetMyCurController(), nearCharacters);

	float minAngle = 1e9;

	TWeakObjectPtr<ACCommonCharacter> candidate;

	for (const auto& elem : nearCharacters)
	{
		const float curAngle = elem.Key;

		bool bCheck = false;
		bCheck |= (InbRight && curAngle > 0.0f);
		bCheck |= (false == InbRight && curAngle < 0.0f);
		bCheck &= (minAngle > abs(curAngle));

		if (bCheck)
		{
			minAngle = abs(curAngle);
			candidate = elem.Value;
		}
	}

	if (candidate.IsValid())
	{
		if (false == bMovingFocus)
		{
			bMovingFocus = true;
			ChangeTarget(candidate.Get());
		}
	}

	bCanMoveFocus = false;
}
