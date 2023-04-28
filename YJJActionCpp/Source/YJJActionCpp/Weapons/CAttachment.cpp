#include "Weapons/CAttachment.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Components/SceneComponent.h"

ACAttachment::ACAttachment()
{
	CHelpers::CreateComponent(this, &Root, "Root");
}

void ACAttachment::BeginPlay()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());

	Super::BeginPlay();
}
