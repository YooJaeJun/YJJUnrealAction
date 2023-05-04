#include "Weapons/CAct.h"
#include "Global.h"
#include "CAttachment.h"
#include "CEquipment.h"
#include "Character/CCommonCharacter.h"
#include "Component/CStateComponent.h"
#include "Component/CMovementComponent.h"

UCAct::UCAct()
{
}

void UCAct::BeginPlay(ACAttachment* InAttachment, UCEquipment* InEquipment, 
	ACCommonCharacter* InOwner, const TArray<FActData>& InActDatas)
{
	Owner = InOwner;
	World = Owner->GetWorld();

	State = CHelpers::GetComponent<UCStateComponent>(Owner.Get());
	Movement = CHelpers::GetComponent<UCMovementComponent>(Owner.Get());

	ActDatas = InActDatas;
}

void UCAct::Act()
{
	State->SetActMode();
}

void UCAct::Begin_Act()
{
	bBeginAct = true;
}

void UCAct::End_Act()
{
	bBeginAct = false;

	State->SetIdleMode();

	Movement->Move();
	Movement->UnFixCamera();
}