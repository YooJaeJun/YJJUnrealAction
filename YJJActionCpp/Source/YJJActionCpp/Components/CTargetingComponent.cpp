#include "Components/CTargetingComponent.h"

#include "CMovementComponent.h"
#include "CStateComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Controller.h"

UCTargetingComponent::UCTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACCommonCharacter>(GetOwner());

	if (!!Owner.Get())
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
	Toggle_Target();
}

void UCTargetingComponent::Toggle_Target()
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
		Owner->GetActorLocation(), Owner->GetActorLocation(),
		TraceDistance, "Targeting",
		false, ignores,
		EDrawDebugTrace::ForOneFrame,
		hitResults, true);

	TArray<TWeakObjectPtr<ACCommonCharacter>> Targets;

	for (const auto& elem : hitResults)
	{
		if (elem.GetActor()->GetClass() != Owner->GetClass())
		{
			Targets.Push(Cast<ACCommonCharacter>(elem.GetActor()));
			bTargeting = true;
		}
	}

	const TWeakObjectPtr<ACCommonCharacter> target = CHelpers::GetNearlyFromAngle(Owner, Targets, Owner->GetMyCurController());
	ChangeTarget(target.Get());
}

void UCTargetingComponent::End_Targeting()
{
	SetVisibleTargetUI(false);

	CheckNull(Target);
	CheckNull(Target->MovementComp);

	Target->MovementComp->UnFixCamera();
	Target = nullptr;
	bTargeting = false;
}

void UCTargetingComponent::ChangeTarget(ACCommonCharacter* InTarget)
{
	if (!!InTarget)
	{
		if (!!Target.Get())
			SetVisibleTargetUI(false);

		Target = InTarget;

		if (!!Target.Get())
			SetVisibleTargetUI(true);

		Target->MovementComp->FixCamera();
	}
	else
	{
		bTargeting = false;
		End_Targeting();
	}
}

void UCTargetingComponent::SetVisibleTargetUI(bool bVisible)
{
	CheckNull(Target.Get());
	CheckNull(Target->TargetingWidgetComp);

	Target->TargetingWidgetComp->SetVisibility(bVisible);
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
	CheckNull(Target.Get());
	CheckNull(Target->StateComp);
	
	if (!!Target->StateComp->IsDeadMode() &&
		!!(Owner->GetDistanceTo(Target.Get()) <= TraceDistance))
	{
		if (nullptr == Controller.Get())
			Controller = Owner->GetMyCurController();

		FRotator controlRotation = Controller->GetControlRotation();
		FRotator ownerToTarget = UKismetMathLibrary::FindLookAtRotation(
			Owner->GetActorLocation(), Target->GetActorLocation());

		if (!!UKismetMathLibrary::EqualEqual_RotatorRotator(controlRotation, ownerToTarget, FinishAngle))
		{
			Controller->SetControlRotation(ownerToTarget);

			if (!!bMovingFocus)
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

	}// IsDeadMode, GetDistanceTo
	else
		End_Targeting();
}

void UCTargetingComponent::ChangeFocus(const bool InRight)
{

}
