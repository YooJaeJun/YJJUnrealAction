#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapons/CWeaponStructures.h"
#include "CWeaponAsset.generated.h"

class ACCommonCharacter;
class ACAttachment;
class UCEquipment;
class UCAct;

UCLASS()
class YJJACTIONCPP_API UCWeaponAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UCWeaponAsset();

	virtual void BeginPlay(ACCommonCharacter* InOwner);

public:
	FORCEINLINE ACAttachment* GetAttachment() const { return Attachment; }
	FORCEINLINE UCEquipment* GetEquipment() const { return Equipment; }
	FORCEINLINE UCAct* GetAct() const { return Act; }

protected:
	UPROPERTY(EditAnywhere)
		TSubclassOf<ACAttachment> AttachmentClass;

	UPROPERTY()
		ACAttachment* Attachment;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCEquipment> EquipmentClass;

	UPROPERTY(EditAnywhere)
		FEquipmentData EquipmentData;

	UPROPERTY()
		UCEquipment* Equipment;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCAct> ActClass;

	UPROPERTY(EditAnywhere)
		TArray<FActData> ActDatas;

	UPROPERTY()
		UCAct* Act;

	UPROPERTY(EditAnywhere)
		TArray<FHitData> HitDatas;
};
