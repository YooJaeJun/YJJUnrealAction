#pragma once

#include "CoreMinimal.h"
#include "Weapons/CSkillWeapon.h"
#include "CSkillAround.generated.h"

class ACCommonCharacter;
class USphereComponent;
class UParticleSystemComponent;

// BP `/Game/Magics/Skill_Around` 와 동형 — ACSkillWeapon(Skill_Magic) 계승 + Sphere·Particle 계층(이미지 컴포넌트 트리).
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACSkillAround : public ACSkillWeapon
{
	GENERATED_BODY()

public:
	ACSkillAround();

	/** Spawn 그래프용 — IsRealRiding 은 UCStateComponent 전용이라 State(UObject)·Self 에 붙인 노드가 깨진다 → Character 에서 상태 컴포넌트를 찾아 판별. */
	UFUNCTION(BlueprintPure, Category = "Around|State", meta = (DisplayName = "Is Character Real Riding"))
	bool Around_IsCharacterRealRiding() const;

	/** 레거시 SetCharacterState 시작부 — Character 가 유효하면 State 필드를 UCStateComponent 로 갱신(BP 변수 타입 Object 유지). */
	UFUNCTION(
		BlueprintCallable,
		Category = "Around|State",
		meta = (DisplayName = "Refresh State Reference From Character Component"))
	void Around_RefreshStateFromCharacterComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Around|Components")
	TObjectPtr<USphereComponent> Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Around|Components")
	TObjectPtr<UParticleSystemComponent> ParticleSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Around|Settings")
	bool Negative = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Around|Settings")
	double Speed = 300.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Around|Settings")
	double Distance = 300.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Around|Default")
	double Angle = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Around|Default")
	TArray<TObjectPtr<ACCommonCharacter>> Hitted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Around|Default")
	FVector SpawnLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Around|Default")
	double ZFactor = 0.0;

	// 레거시 BP 무지정 UObject — 보통 UCStateComponent 참조용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Around|Default")
	TObjectPtr<UObject> State;
};
