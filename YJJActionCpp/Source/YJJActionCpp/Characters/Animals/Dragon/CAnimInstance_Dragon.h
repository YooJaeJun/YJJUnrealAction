#pragma once
#include "CoreMinimal.h"
#include "Characters/Animals/CAnimInstance_Animal.h"
#include "CAnimInstance_Dragon.generated.h"

class ACDragon_AI;
class ACDragonSkill;

UCLASS()
class YJJACTIONCPP_API UCAnimInstance_Dragon : public UCAnimInstance_Animal
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	TWeakObjectPtr<ACDragon_AI> OwnerDragon;
	TWeakObjectPtr<ACDragonSkill> Skill;
};
