#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Component/CStateComponent.h"	// Dynamic Multicast Delegate
#include "CCommonCharacter.generated.h"

class UCMovementComponent;

UCLASS()
class YJJACTIONCPP_API ACCommonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
		UCMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
		UCStateComponent* StateComponent;

	UPROPERTY(EditAnywhere)
		FString Name;

public:
	FORCEINLINE const FString& GetName() const { return Name; }

public:
	ACCommonCharacter();

protected:
	virtual void BeginPlay() override;
};
