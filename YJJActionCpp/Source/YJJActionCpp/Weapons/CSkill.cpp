#include "Weapons/CSkill.h"

#include "AITypes.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"

UCSkill::UCSkill()
{
}

void UCSkill::BeginPlay(
    TWeakObjectPtr<ACCommonCharacter> InOwner, 
    TWeakObjectPtr<ACAttachment> InAttachment,
    TWeakObjectPtr<UCAct> InAct)
{
    Owner = InOwner;
    Attachment = InAttachment;
    Act = InAct;

    StateComp = YJJHelpers::GetComponent<UCStateComponent>(Owner.Get());
    MovementComp = YJJHelpers::GetComponent<UCMovementComponent>(Owner.Get());
}

void UCSkill::Begin_Skill_Implementation()
{
    bInAction = true;
}

void UCSkill::End_Skill_Implementation()
{
    bInAction = false;
}