#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapons/CWeaponStructures.h"
#include "CWeaponAsset.generated.h"

class ACAttachment;
class UCEquipment;
class UCAct;
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

public:
	FORCEINLINE ACAttachment* GetAttachment() const { return Attachment; }
	FORCEINLINE UCEquipment* GetEquipment() const { return Equipment; }
	FORCEINLINE UCAct* GetAct() const { return Act; }

public:
	UCWeaponAsset();

	void BeginPlay(ACCommonCharacter* InOwner);
};
