#include "Weapons/CSkill.h"

#include "AITypes.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CCamComponent.h"

UCSkill::UCSkill()
{
}

void UCSkill::BeginPlay(
    TWeakObjectPtr<ACCommonCharacter> InOwner, 
    ACAttachment* InAttachment,
    UCAct* InAct)
{
    Owner = InOwner;
    Attachment = InAttachment;
    Act = InAct;

    StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
    MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());
    CamComp = YJJHelpers::GetComponent<UCCamComponent>(Owner.Get());
}

void UCSkill::Pressed()
{
    bInAction = true;
}

void UCSkill::Released()
{
    bInAction = false;
}