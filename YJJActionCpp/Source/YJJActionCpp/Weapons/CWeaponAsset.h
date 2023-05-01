#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapons/CWeaponStructures.h"
#include "CWeaponAsset.generated.h"

class ACAttachment;
class UCEquipment;
class ACCommonCharacter;

UCLASS()
class YJJACTIONCPP_API UCWeaponAsset : public UDataAsset
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
		TSubclassOf<ACAttachment> AttachmentClass;

	UPROPERTY()
		ACAttachment* Attachment;

	UPROPERTY(EditAnywhere)
		FEquipmentData EquipmentData;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCEquipment> EquipmentClass;

	UPROPERTY()
		UCEquipment* Equipment;

public:
	FORCEINLINE ACAttachment* GetAttachment() { return Attachment; }
	FORCEINLINE UCEquipment* GetEquipment() { return Equipment; }

public:
	UCWeaponAsset();

	void BeginPlay(ACCommonCharacter* InOwner);
};
