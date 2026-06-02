#pragma once

#include "CoreMinimal.h"
#include "Weapons/CMagic.h"
#include "Weapons/CWeapon.h"
#include "Components/SceneComponent.h"
#include "CSkillWeaponActors.generated.h"

class ACCommonCharacter;

// BP Skill_Weapon / Skill_Magic — ChildActor 또는 스폰 시 Owner 가 Magic(ACMagic) 또는 Weapon(ACWeapon) 이면
// BeginPlay 에서 아래 참조가 채워진다. 블루프린트에 동일 이름(Character/Weapon/Magic) 변수를 만들면 숨김·복제 충돌이 나므로 두지 않는다.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACSkillWeapon : public AActor
{
	GENERATED_BODY()

public:
	ACSkillWeapon();

protected:
	/** 레거시 BP Skill_Weapon / Skill_Around 의 DefaultSceneRoot 와 동일한 이름으로 자식 무기 계열이 붙는다(Sphere 등). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SkillWeapon|Components")
	TObjectPtr<USceneComponent> DefaultSceneRoot;

	virtual void BeginPlay() override;

public:
	// BeginPlay 채우기 전 수동 에디터 대입 허용(레거시). 스킬 BP는 중복 변수 대신 여기 연결 또는 그래프 제거 후 상속 Getter만 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<ACMagic> Magic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<ACWeapon> Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<ACCommonCharacter> Character;
};

// BP `/Game/Magics/Skill_Magic` — 부모 `ACSkillMagic` 에서 상속 프로퍼티 Character/Magic/Weapon 만 쓸 것(블프 동명 변수 금지). BeginPlay 순서 상 C++ 채운 뒤 BP Event 가능.
// 무기 블프 `/Game/Weapons/Weapon` 은 반드시 `ACWeapon` 계통이어야 `Character` 속성 노드가 유효하다(`ACOwnerActor::Character`).
UCLASS(
	Blueprintable,
	meta=(
		DisplayName="CSkillMagic",
		BlueprintDescription="AActor 가 아니라 ACSkillMagic(ACSkillWeapon) 을 부모로 지정해야 BeginPlay 에서 Owner(Magic/Weapon) 기준 Character 등이 채워진다."))
class YJJACTIONCPPUE5_API ACSkillMagic : public ACSkillWeapon
{
	GENERATED_BODY()

public:
	ACSkillMagic() = default;
};
