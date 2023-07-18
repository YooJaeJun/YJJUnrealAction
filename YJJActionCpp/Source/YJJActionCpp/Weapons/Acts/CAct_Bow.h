#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "Weapons/CWeaponStructures.h"
#include "CAct_Bow.generated.h"

class ACAttachment;
class UCEquipment;
class ACCommonCharacter;
class ACArrow;
class USkeletalMeshComponent;
class UPoseableMeshComponent;

UCLASS(Blueprintable)
class YJJACTIONCPP_API UCAct_Bow : public UCAct
{
	GENERATED_BODY()

public:
	UCAct_Bow();

	virtual void BeginPlay(
		TWeakObjectPtr<ACCommonCharacter> InOwner,
		TWeakObjectPtr<ACAttachment> InAttachment,
		TWeakObjectPtr<UCEquipment> InEquipment,
		const TArray<FActData>& InDoActionData,
		const TArray<FHitData>& InHitData
	) override;

public:
	virtual void OnBeginEquip() override;
	virtual void OnUnequip() override;

	virtual void Act() override;
	virtual void Begin_Act() override;
	virtual void End_Act() override;

	virtual void Tick(float InDeltaTime) override;

public:
	void End_BowString();

private:
	void CreateArrow();
	ACArrow* GetAttachedArrow();

private:
	UFUNCTION()
		void OnArrowHit(AActor* InCauser, ACCommonCharacter* InOther);

	UFUNCTION()
		void OnArrowEndPlay(ACArrow* InDestroyer);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Arrow")
		TSubclassOf<ACArrow> ArrowClass;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		UPoseableMeshComponent* PoseableMesh;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
		TArray<ACArrow*> Arrows;

private:
	FVector OriginLocation;
	bool bAttachedString = true;

	// CAnimInstance_Bow.h 의 Bend를 참조하고 있다.
	float* Bending = nullptr;
	const bool* bEquipped;
};