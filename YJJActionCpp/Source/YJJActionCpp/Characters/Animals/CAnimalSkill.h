#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAnimalSkill.generated.h"

UCLASS()
class YJJACTIONCPP_API ACAnimalSkill : public AActor
{
	GENERATED_BODY()
	
public:	
	ACAnimalSkill();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere)
		bool bRotating;
};
