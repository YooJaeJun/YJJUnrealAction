#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "CAct_Warp.generated.h"

class ACAttachment;
class UCEquipment;
class ACCommonCharacter;
class AController;
class UDecalComponent;
class UFXSystemAsset;
struct FActData;
struct FHitData;

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCAct_Warp : public UCAct
{
	GENERATED_BODY()

public:
	UCAct_Warp();

	virtual void BeginPlay(
		TWeakObjectPtr<ACCommonCharacter> InOwner,
		TWeakObjectPtr<ACAttachment> InAttachment,
		TWeakObjectPtr<UCEquipment> InEquipment,
		const TArray<FActData>& InActData,
		const TArray<FHitData>& InHitData) override;

	virtual void Tick(float InDeltaTime) override;

	virtual void Act() override;
	virtual void Begin_Act() override;

private:
	bool GetCursorLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const;

private:
	TWeakObjectPtr<UFXSystemAsset> Effect;
	TWeakObjectPtr<UDecalComponent> Decal;
	TWeakObjectPtr<AController> Controller;
	FVector MoveToLocation;
};
