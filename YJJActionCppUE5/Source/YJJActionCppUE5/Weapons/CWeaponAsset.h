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
	FORCEINLINE constexpr CEMagicType GetMagicType() const { return MagicType; }
	FORCEINLINE constexpr ACAttachment* GetAttachment() const { return Attachment; }
	FORCEINLINE constexpr UCEquipment* GetEquipment() const { return Equipment; }
	FORCEINLINE constexpr UCAct* GetAct() const { return Act; }
	UCSkill* GetSkill(const int32 SkillIndex) const;

protected:
	UPROPERTY(EditAnywhere)
	CEWeaponType Type;

	// 마법 전용 DA 는 MagicType 을 채우고 Type 은 Unarmed 로 둔다. BeginPlay 가 맵을 둘로 나눈다.
	UPROPERTY(EditAnywhere)
	CEMagicType MagicType = CEMagicType::Unarmed;

	UPROPERTY(EditAnywhere)
	TSubclassOf<ACAttachment> AttachmentClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCEquipment> EquipmentClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCAct> ActClass;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<UCSkill>> SkillClasses;


	UPROPERTY(EditAnywhere)
	FEquipData EquipmentData;

	UPROPERTY(EditAnywhere)
	TArray<FDoActionData> ActDatas;

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