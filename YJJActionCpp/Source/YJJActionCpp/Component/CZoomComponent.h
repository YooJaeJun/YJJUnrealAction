#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CZoomComponent.generated.h"

class ACPlayableCharacter;

USTRUCT()
struct FZoomData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		float ZoomSpeed = 40;

	UPROPERTY(EditAnywhere)
		float MinRange = 100;

	UPROPERTY(EditAnywhere)
		float MaxRange = 600;

	UPROPERTY(EditAnywhere)
		float InterpSpeed = 5;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCZoomComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
		float Zooming;

	UPROPERTY(EditAnywhere)
		FZoomData ZoomData;

public:	
	UCZoomComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION()
		void Zoom(const float axis);

private:
	TWeakObjectPtr<ACPlayableCharacter> Owner;
};
