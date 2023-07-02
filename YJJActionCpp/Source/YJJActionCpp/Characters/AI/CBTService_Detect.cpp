#include "Characters/AI/CBTService_Detect.h"
#include "Global.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/AI/CAIController.h"
#include "Components/CCharacterInfoComponent.h"

UCBTService_Detect::UCBTService_Detect()
{
	NodeName = TEXT("Detect");
	Interval = 1.0f;
}

void UCBTService_Detect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	//const TWeakObjectPtr<ACCommonCharacter> owner = 
	//	Cast<ACCommonCharacter>(OwnerComp.GetAIOwner()->GetPawn());
	//CheckNull(owner);

	//const TWeakObjectPtr<UWorld> world = owner->GetWorld();
	//CheckNull(world);

	//FVector currentLocation = owner->GetActorLocation();

	//const TArray<AActor*> ignores{ owner.Get() };
	//TArray<FHitResult> hitResults;

	//const bool bResult = UKismetSystemLibrary::SphereTraceMultiByProfile(GetWorld(),
	//	owner->GetActorLocation(),
	//	owner->GetActorLocation(),
	//	traceDistance,
	//	"Pawn",
	//	false,
	//	ignores,
	//	EDrawDebugTrace::None,
	//	hitResults,
	//	false);

	//if (false == bResult)
	//{
	//	OwnerComp.GetBlackboardComponent()->SetValueAsObject(ACAIController::Target, nullptr);
	//	return;
	//}

	//for (const auto& hitResult : hitResults)
	//{
	//	TWeakObjectPtr<ACCommonCharacter> target = Cast<ACCommonCharacter>(hitResult.GetActor());
	//
	//	if (!!target.Get() &&
	//		false == target->CharacterInfoComp->IsSameGroup(owner))
	//	{
	//		OwnerComp.GetBlackboardComponent()->SetValueAsObject(ACAIController::Target, target.Get());
	//	}
	//}
}
