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

	const UCWeaponAsset& CopyDeep(
		const UCWeaponAsset& InOther, 
		const TWeakObjectPtr<ACCommonCharacter> Owner);

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

	UPROPERTY()
		UCEquipment* Equipment;

	UPROPERTY(EditAnywhere)
		FEquipmentData EquipmentData;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCAct> ActClass;

	UPROPERTY()
		UCAct* Act;

	UPROPERTY(EditAnywhere)
		TArray<FActData> ActDatas;

	UPROPERTY(EditAnywhere)
		TArray<FHitData> HitDatas;
};
