#include "Weapons/CAct.h"
#include "Global.h"
#include "CAttachment.h"
#include "CEquipment.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CCamComponent.h"

UCAct::UCAct()
{
}

void UCAct::BeginPlay(
	TWeakObjectPtr<ACCommonCharacter> InOwner,
	TWeakObjectPtr<ACAttachment> InAttachment,
	TWeakObjectPtr<UCEquipment> InEquipment,
	const TArray<FDoActionData>& InActDatas, 
	const TArray<FHitData>& InHitDatas)
{
	Owner = InOwner;
	World = Owner->GetWorld();

	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
	MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());
	CamComp = YJJHelpers::GetComponent<UCCamComponent>(Owner.Get());
	
	for (int i = 0; i < InActDatas.Num(); i++)
		ActDatas.Emplace(InActDatas[i]);

	for (int i = 0; i < InHitDatas.Num(); i++)
		HitDatas.Emplace(InHitDatas[i]);
}

void UCAct::Act()
{
	if (false == StateComp.IsValid())
	{
		const AActor* const ownerActor = Owner.Get();
		CLog::Log(FString::Printf(
			TEXT("[입력][Action] StateComp 없음 — Owner=%s"),
			IsValid(ownerActor) ? *ownerActor->GetName() : TEXT("(null)")));
		return;
	}

	StateComp->SetActMode();
}

void UCAct::Begin_Act()
{
	bBeginAct = true;
}

void UCAct::End_Act()
{
	bBeginAct = false;

	if (StateComp.IsValid())
		StateComp->SetIdleMode();

	if (MovementComp.IsValid())
		MovementComp->Move();

	if (CamComp.IsValid())
	{
		CamComp->DisableFixedCamera();
		CamComp->DisableControlRotation();
	}
}