#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Weapons/CWeaponStructures.h"
#include "Commons/CEnums.h"
#include "CWeaponAsset.generated.h"

class ACCommonCharacter;
class ACAttachment;
class UCEquipment;
class UCAct;
class UCSkill;

UCLASS()
class YJJACTIONCPP_API UCWeaponAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UCWeaponAsset();

	virtual void BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner);

	const UCWeaponAsset& DeepCopy(
		const UCWeaponAsset& InOther, 
		const TWeakObjectPtr<ACCommonCharacter> Owner);

public:
	FORCEINLINE constexpr CEWeaponType GetType() const { return Type; }
	FORCEINLINE constexpr ACAttachment* GetAttachment() const { return Attachment; }
	FORCEINLINE constexpr UCEquipment* GetEquipment() const { return Equipment; }
	FORCEINLINE constexpr UCAct* GetAct() const { return Act; }
	FORCEINLINE constexpr UCSkill* GetSkill() const { return Skill; }

protected:
	UPROPERTY(EditAnywhere)
		CEWeaponType Type;

	UPROPERTY(EditAnywhere)
		TSubclassOf<ACAttachment> AttachmentClass;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCEquipment> EquipmentClass;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCAct> ActClass;

	UPROPERTY(EditAnywhere)
		TSubclassOf<UCSkill> SkillClass;


	UPROPERTY(EditAnywhere)
		FEquipmentData EquipmentData;

	UPROPERTY(EditAnywhere)
		TArray<FActData> ActDatas;

	UPROPERTY(EditAnywhere)
		TArray<FHitData> HitDatas;


	UPROPERTY()
		ACAttachment* Attachment;

	UPROPERTY()
		UCEquipment* Equipment;

	UPROPERTY()
		UCAct* Act;

	UPROPERTY()
		UCSkill* Skill;
};
