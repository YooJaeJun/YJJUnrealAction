#include "Weapons/CAct.h"
#include "Global.h"
#include "CAttachment.h"
#include "CEquipment.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CCameraComponent.h"

UCAct::UCAct()
{
}

void UCAct::BeginPlay(
	TWeakObjectPtr<ACCommonCharacter> InOwner,
	TWeakObjectPtr<ACAttachment> InAttachment,
	TWeakObjectPtr<UCEquipment> InEquipment,
	const TArray<FActData>& InActDatas, 
	const TArray<FHitData>& InHitDatas)
{
	Owner = InOwner;
	World = Owner->GetWorld();

	StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
	MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());
	CameraComp = YJJHelpers::GetComponent<UCCameraComponent>(Owner.Get());
	
	for (int i = 0; i < InActDatas.Num(); i++)
		ActDatas.Emplace(InActDatas[i]);

	for (int i = 0; i < InHitDatas.Num(); i++)
		HitDatas.Emplace(InHitDatas[i]);
}

void UCAct::Act()
{
	CheckNull(StateComp);
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

	if (CameraComp.IsValid())
		CameraComp->UnFixCamera();
}