#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "UObject/Interface.h"
#include "ICombatActionHost.generated.h"

/**
 * CDA(UCAct) 경로와 MainWeapon(ACWeapon) 경로의 공통 액션·콜리전 디스패치 표면.
 * CWeaponComponent 는 이 인터페이스를 우선 호출하고, 없으면 ProcessEvent 로 폴백한다.
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatActionHost : public UInterface
{
	GENERATED_BODY()
};

class YJJACTIONCPPUE5_API ICombatActionHost
{
	GENERATED_BODY()

public:
	virtual void Host_DoAction(CEAttackType InAttackType, int32 InSkillIndex) {}
	virtual void Host_BeginDoAction(CEAttackType InAttackType) {}
	virtual void Host_EndDoAction(CEAttackType InAttackType) {}

	virtual void Host_OnCollisions() {}
	virtual void Host_OffCollisions() {}
	virtual void Host_OnBoxCollisions() {}

	virtual void Host_EnableCombo() {}
	virtual void Host_DisableCombo() {}

	virtual void Host_Skill(int32 InSkillIndex) {}
	virtual void Host_EndSkill() {}
};
