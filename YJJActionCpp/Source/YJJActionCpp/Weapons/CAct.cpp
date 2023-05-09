#include "Weapons/CAct.h"
#include "Global.h"
#include "CAttachment.h"
#include "CEquipment.h"
#include "Character/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"

UCAct::UCAct()
{
}

void UCAct::BeginPlay(ACAttachment* InAttachment, UCEquipment* InEquipment, 
	ACCommonCharacter* InOwner, const TArray<FActData>& InActDatas,
	const TArray<FHitData>& InHitDatas)
{
	Owner = InOwner;
	World = Owner->GetWorld();

	StateComp = CHelpers::GetComponent<UCStateComponent>(Owner.Get());
	MovementComp = CHelpers::GetComponent<UCMovementComponent>(Owner.Get());

	ActDatas = InActDatas;
	HitDatas = InHitDatas;
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