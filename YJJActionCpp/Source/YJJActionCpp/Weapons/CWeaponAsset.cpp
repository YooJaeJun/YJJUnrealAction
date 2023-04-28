#include "Weapons/CWeaponAsset.h"
#include "Global.h"
#include "CAttachment.h"
#include "Character/CCommonCharacter.h"
#include "GameFramework/Character.h"

UCWeaponAsset::UCWeaponAsset()
{
	AttachmentClass = ACAttachment::StaticClass();
}

void UCWeaponAsset::BeginPlay(ACCommonCharacter* InOwner)
{
	if (!!AttachmentClass)
	{
		FActorSpawnParameters params;
		params.Owner = Cast<AActor>(InOwner);

		InOwner->GetWorld()->SpawnActor<ACAttachment>(AttachmentClass, params);
	}
}