#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Weapons/CWeaponStructures.h"
#include "CAct.generated.h"

class ACAttachment;
class UCEquipment;
class ACCommonCharacter;
class UWorld;
class UCMovementComponent;
class UCStateComponent;

UCLASS()
class YJJACTIONCPP_API UCAct : public UObject
{
	GENERATED_BODY()

public:
	UCAct();

	virtual void BeginPlay(
		TWeakObjectPtr<ACCommonCharacter> InOwner,
		TWeakObjectPtr<ACAttachment> InAttachment,
		TWeakObjectPtr<UCEquipment> InEquipment,
		const TArray<FActData>& InActDatas,
		const TArray<FHitData>& InHitDatas);

public:
	UFUNCTION()
		virtual void OnAttachmentBeginCollision() {}

	UFUNCTION()
		virtual void OnAttachmentEndCollision() {}

	UFUNCTION()
		virtual void OnAttachmentBeginOverlap(
			ACCommonCharacter* InAttacker,
			AActor* InAttackCauser,
			ACCommonCharacter* InOther) {}

	UFUNCTION()
		virtual void OnAttachmentEndOverlap(
			ACCommonCharacter* InAttacker,
			ACCommonCharacter* InOther) {}

public:
	virtual void Act();
	virtual void Begin_Act();
	virtual void End_Act();

protected:
	bool bBeginAct = false;

	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UWorld> World;

	TWeakObjectPtr<UCMovementComponent> MovementComp;
	TWeakObjectPtr<UCStateComponent> StateComp;

	TArray<FActData> ActDatas;
	TArray<FHitData> HitDatas;
};
