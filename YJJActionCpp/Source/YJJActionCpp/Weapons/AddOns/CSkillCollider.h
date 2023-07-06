#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSkillCollider.generated.h"

UCLASS()
class YJJACTIONCPP_API ACSkillCollider : public AActor
{
	GENERATED_BODY()
	
public:	
	ACSkillCollider();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
