#include "Characters/AI/CBTService_Detect.h"
#include "Global.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/AI/CAIController.h"

UCBTService_Detect::UCBTService_Detect()
{
	NodeName = TEXT("Detect");
	Interval = 1.0f;
}

void UCBTService_Detect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const TWeakObjectPtr<APawn> owner = OwnerComp.GetAIOwner()->GetPawn();
	CheckNull(owner);

	const TWeakObjectPtr<UWorld> world = owner->GetWorld();
	CheckNull(world);

	FVector currentLocation = owner->GetActorLocation();

	const TArray<AActor*> ignores{ owner.Get() };
	TArray<FHitResult> hitResults;

	const bool bResult = UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(),
		owner->GetActorLocation(),
		owner->GetActorLocation(),
		traceDistance,
		"Pawn",
		false,
		ignores,
		EDrawDebugTrace::None,
		hitResults,
		false);

	if (!!bResult)
	{
		for (const auto& hitResult : hitResults)
		{
			TWeakObjectPtr<ACCommonCharacter> target = Cast<ACCommonCharacter>(hitResult.GetActor());

			if (!!target.Get() && 
				target->GetMyCurController()->IsPlayerController())
				OwnerComp.GetBlackboardComponent()->SetValueAsObject(ACAIController::Target, target.Get());
		}
	}//bResult
	else
		OwnerComp.GetBlackboardComponent()->SetValueAsObject(ACAIController::Target, nullptr);
}
