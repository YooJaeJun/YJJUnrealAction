#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAttachment.h"
#include "CAttachment_Bow.generated.h"

class ACArrow;
class USkeletalMeshComponent;
class UPoseableMeshComponent;

UCLASS()
class YJJACTIONCPPUE5_API ACAttachment_Bow : public ACAttachment
{
	GENERATED_BODY()

public:
	ACAttachment_Bow();

protected:
	virtual void BeginPlay() override;

public:
	virtual void OnBeginEquip_Implementation() override;
	virtual void OnUnequip_Implementation() override;

public:
	TSharedPtr<float> GetBend() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "View")
		FVector2D ViewPitchRange = FVector2D(-40, +30);

private:
	UPROPERTY(VisibleAnywhere)
		TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

	UPROPERTY(VisibleAnywhere)
		TObjectPtr<UPoseableMeshComponent> PoseableMesh;

private:
	FVector2D OriginViewPitchRange;
};