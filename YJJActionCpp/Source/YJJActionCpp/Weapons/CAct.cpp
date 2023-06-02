#include "Weapons/CAct.h"
#include "Global.h"
#include "CAttachment.h"
#include "CEquipment.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"

UCAct::UCAct()
{
}

void UCAct::BeginPlay(ACCommonCharacter* InOwner, 
	ACAttachment* InAttachment, 
	UCEquipment* InEquipment, 
	const TArray<FActData>& InActDatas, 
	const TArray<FHitData>& InHitDatas)
{
	Owner = InOwner;
	World = Owner->GetWorld();

	StateComp = CHelpers::GetComponent<UCStateComponent>(Owner.Get());
	MovementComp = CHelpers::GetComponent<UCMovementComponent>(Owner.Get());
	
	for (int i = 0; i < InActDatas.Num(); i++)
		ActDatas.Emplace(InActDatas[i]);

	for (int i = 0; i < InHitDatas.Num(); i++)
		HitDatas.Emplace(InHitDatas[i]);
}

void UCAct::Act()
{
	StateComp->SetActMode();
}

void UCAct::Begin_Act()
{
	bBeginAct = true;
}

void UCAct::End_Act()
{
	bBeginAct = false;

	StateComp->SetIdleMode();

	MovementComp->Move();
	MovementComp->UnFixCamera();
}