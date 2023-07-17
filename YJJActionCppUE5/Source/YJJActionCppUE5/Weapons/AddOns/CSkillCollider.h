#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSkillCollider.generated.h"

class ACCommonCharacter;

UCLASS()
class YJJACTIONCPPUE5_API ACSkillCollider : public AActor
{
	GENERATED_BODY()

protected:
	TArray<TWeakObjectPtr<ACCommonCharacter>> Hitted;
};