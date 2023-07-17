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
}
