#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CTargetingComponent.generated.h"

class ACPlayableCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCTargetingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCTargetingComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION()
		void InputAction_Targeting();

	UFUNCTION()
		void ChangeFocus(const bool InRight);

public:
	UPROPERTY(VisibleAnywhere)
		bool IsTargeting;

private:
	TWeakObjectPtr<ACPlayableCharacter> Owner;
};
