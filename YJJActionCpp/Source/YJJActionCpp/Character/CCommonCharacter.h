#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCommonCharacter.generated.h"

class UCMovementComponent;
class UCStateComponent;

UCLASS()
class YJJACTIONCPP_API ACCommonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACCommonCharacter();

	UPROPERTY(VisibleAnywhere)
		UCStateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere)
		UCMovementComponent* MovementComponent;

protected:
	virtual void BeginPlay() override;
};
