#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CSkillWeapon.generated.h"

class ACCommonCharacter;

// BP Magic / Weapon 이 공유하는 스킬 주체 캐릭터 슬롯.
UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACSkillContextProvider : public AActor
{
	GENERATED_BODY()

public:
	// Magic/Weapon BP BeginPlay 에서 Owner 를 Cast 해 채우므로 블루프린트에서 쓰기 허용한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<ACCommonCharacter> Character;
};

// BP `/Game/Magics/Magic` 가 달도록 하는 마커 부모(첫 번째 Cast 분기).
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACMagicSkillContext : public ACSkillContextProvider
{
	GENERATED_BODY()
};

// BP `/Game/Weapons/Weapon` 가 달도록 하는 마커 부모(두 번째 Cast 분기).
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponSkillContext : public ACSkillContextProvider
{
	GENERATED_BODY()
};

// BP `Skill_Weapon` — Owner 가 마법·무기 중 어디에 붙었는지에 따라 Character 캐시.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACSkillWeapon : public AActor
{
	GENERATED_BODY()

public:
	ACSkillWeapon();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<ACMagicSkillContext> Magic;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<ACWeaponSkillContext> Weapon;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TObjectPtr<ACCommonCharacter> Character;
};
