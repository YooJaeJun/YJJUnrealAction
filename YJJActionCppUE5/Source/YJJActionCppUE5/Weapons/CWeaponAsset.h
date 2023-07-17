#pragma once

#include "CoreMinimal.h"
#include "Global.h"
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
class YJJACTIONCPPUE5_API UCWeaponAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UCWeaponAsset();

	const UCWeaponAsset& DeepCopy(
		const UCWeaponAsset& InOther,
		const TWeakObjectPtr<ACCommonCharacter> Owner);

	virtual void BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner);

public:
	FORCEINLINE constexpr CEWeaponType GetType() const { return Type; }
	FORCEINLINE constexpr ACAttachment* GetAttachment() const { return Attachment; }
	FORCEINLINE constexpr UCEquipment* GetEquipment() const { return Equipment; }
	FORCEINLINE constexpr UCAct* GetAct() const { return Act; }
	UCSkill* GetSkill(const int32 SkillIndex) const;

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
		TArray<TSubclassOf<UCSkill>> SkillClasses;


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
		TArray<UCSkill*> Skills;
};