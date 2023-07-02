#pragma once
#include "CoreMinimal.h"
#include "Characters/Animals/CAnimal_AI.h"
#include "CDragon_AI.generated.h"

class UCFlyComponent;
class ACDragonSkill;

UCLASS()
class YJJACTIONCPP_API ACDragon_AI : public ACAnimal_AI
{
	GENERATED_BODY()

public:
	ACDragon_AI();

protected:
	virtual void BeginPlay() override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void Landed(const FHitResult& Hit) override;

public:
	UPROPERTY(VisibleAnywhere)
		UCFlyComponent* FlyComp;
};
