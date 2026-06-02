#include "Characters/AI/CBTService_TeleportProcess.h"

#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Characters/Animals/Dragon/Weapon/CDragonWeapon.h"
#include "Components/TextRenderComponent.h"
#include "Global.h"
#include "Utilities/YJJHelpers.h"

UCBTService_TeleportProcess::UCBTService_TeleportProcess()
{
	NodeName = TEXT("BTService_TeleportProcess");
	bCreateNodeInstance = true;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	bNotifyTick = true;

	Interval = 0.0f;
	RandomDeviation = 0.0f;
}

void UCBTService_TeleportProcess::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	const UBlackboardData* const blackboardAsset = Asset.BlackboardAsset;
	if (nullptr == blackboardAsset)
	{
		return;
	}

	TeleportProcessKey.ResolveSelectedKey(*blackboardAsset);
}

void UCBTService_TeleportProcess::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	OwningAi = OwnerComp.GetAIOwner();
	CachedDragon = Cast<ACDragon>(IsValid(OwningAi) ? OwningAi->GetPawn() : nullptr);
	RefreshDragonWeaponTeleportProcess();
}

void UCBTService_TeleportProcess::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	OwningAi = nullptr;
	CachedDragon = nullptr;
	TeleportProcess = CETeleportProcessType::None;
}

void UCBTService_TeleportProcess::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
	float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (false == IsValid(CachedDragon))
	{
		OwningAi = OwnerComp.GetAIOwner();
		CachedDragon = Cast<ACDragon>(IsValid(OwningAi) ? OwningAi->GetPawn() : nullptr);
		RefreshDragonWeaponTeleportProcess();
	}

	PushTeleportProcess(OwnerComp);
}

void UCBTService_TeleportProcess::RefreshDragonWeaponTeleportProcess()
{
	TeleportProcess = CETeleportProcessType::None;

	if (false == IsValid(CachedDragon))
	{
		return;
	}

	const ACDragonWeapon* const dragonWeapon =
		Cast<ACDragonWeapon>(CachedDragon->DragonWeapon.Get());
	if (false == IsValid(dragonWeapon))
	{
		CLog::Log(FString::Printf(
			TEXT("[BTService_TeleportProcess] DragonWeapon 없음 — TeleportProcess=None 유지 Dragon=%s"),
			*CachedDragon->GetName()));
		return;
	}

	TeleportProcess = dragonWeapon->TeleportProcessType;
}

void UCBTService_TeleportProcess::PushTeleportProcess(UBehaviorTreeComponent& OwnerComp)
{
	const uint8 processByte = static_cast<uint8>(TeleportProcess);
	WriteBlackboardTeleportProcess(OwnerComp, processByte);
	TryWriteStateDebugText(CachedDragon.Get(), TeleportProcess);
}

void UCBTService_TeleportProcess::WriteBlackboardTeleportProcess(
	UBehaviorTreeComponent& OwnerComp,
	uint8 InProcessByte) const
{
	if (false == TeleportProcessKey.SelectedKeyName.IsValid())
	{
		return;
	}

	UBlackboardComponent* const blackboard = OwnerComp.GetBlackboardComponent();
	if (false == IsValid(blackboard))
	{
		CLog::Log(TEXT("[BTService_TeleportProcess] Blackboard 없음 — TeleportProcessKey 기록 생략"));
		return;
	}

	blackboard->SetValueAsEnum(TeleportProcessKey.SelectedKeyName, InProcessByte);
}

void UCBTService_TeleportProcess::TryWriteStateDebugText(
	ACDragon* TextSourceDragon,
	CETeleportProcessType InProcess) const
{
	if ((false == bUpdateStateTextRender) || (false == IsValid(TextSourceDragon)))
	{
		return;
	}

	UTextRenderComponent* const stateTextRender = TextSourceDragon->GetYJJStateTextRenderComponent();
	if (false == IsValid(stateTextRender))
	{
		return;
	}

	const FString labelLocal = YJJHelpers::ConvertEnumToString(InProcess);
	stateTextRender->SetText(FText::FromString(labelLocal));
}
