#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CPlacementComponent.generated.h"

class ACPlayerController;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCPlacementComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCPlacementComponent();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Placement")
	void StartPlacement(FName InItemID);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void ConfirmPlacement();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void CancelPlacement();

	FORCEINLINE bool IsPreviewValid() const { return bPreviewValid; }

private:
	bool TracePlacement(FTransform& OutTransform) const;
	ACPlayerController* GetOwningPlayerController() const;
	void SetPreviewTransform(const FTransform& InTransform);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Placement")
	TSubclassOf<AActor> PreviewActorClass;

	UPROPERTY(EditDefaultsOnly, Category = "Placement", meta = (ClampMin = "0"))
	float TraceDistance = 1200.0f;

	UPROPERTY(Transient)
	TObjectPtr<AActor> PreviewActor;

	FTransform LastPreviewTransform = FTransform::Identity;
	FName PendingItemID = NAME_None;
	bool bPreviewValid = false;
};
