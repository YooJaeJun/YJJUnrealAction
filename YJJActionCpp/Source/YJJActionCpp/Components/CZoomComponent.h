#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CZoomComponent.generated.h"

class ACPlayableCharacter;
class ACAnimal_AI;
class UCTargetingComponent;
class UCMovementComponent;

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
	UCZoomComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION()
		void InputAxis_Zoom(const float InAxis);

public:
	UPROPERTY(EditAnywhere)
		FZoomData ZoomData;

	UPROPERTY(VisibleAnywhere)
		float Zooming;

	UPROPERTY(VisibleAnywhere)
		float TargetArmLength;

private:
	TWeakObjectPtr<ACPlayableCharacter> OwnerPlayer;
	TWeakObjectPtr<ACAnimal_AI> OwnerAnimal;
	TWeakObjectPtr<UCTargetingComponent> TargetingComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;
};
