#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CCharacter.generated.h"

class UCMovingComponent;
class UCStateComponent;

UCLASS()
class YJJACTIONCPP_API ACCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACCharacter();

	UPROPERTY(VisibleAnywhere)
		UCStateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere)
		UCMovingComponent* MovingComponent;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
};
