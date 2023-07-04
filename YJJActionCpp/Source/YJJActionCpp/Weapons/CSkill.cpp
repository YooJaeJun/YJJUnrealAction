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

    StateComp = CHelpers::GetComponent<UCStateComponent>(Owner.Get());
    MovementComp = CHelpers::GetComponent<UCMovementComponent>(Owner.Get());
}
