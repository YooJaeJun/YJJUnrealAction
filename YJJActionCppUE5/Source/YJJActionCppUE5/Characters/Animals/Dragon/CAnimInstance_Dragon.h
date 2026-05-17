#pragma once
#include "CoreMinimal.h"
#include "Characters/Animals/CAnimInstance_Animal.h"
#include "CAnimInstance_Dragon.generated.h"

class ACDragon;
class ACDragonSkill;

UCLASS()
class YJJACTIONCPPUE5_API UCAnimInstance_Dragon : public UCAnimInstance_Animal
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	// 레거시 ABP_Dragon — DragonWeapon C++ 포팅 전까지 Anim BP 에서 설정 가능.
	UPROPERTY(BlueprintReadWrite, Category = "Dragon", meta = (DisplayName = "Is Firing Flying"))
	bool bIsFiringFlying = false;

private:
	TWeakObjectPtr<ACDragon> OwnerDragon;
	TWeakObjectPtr<ACDragonSkill> Skill;
};
