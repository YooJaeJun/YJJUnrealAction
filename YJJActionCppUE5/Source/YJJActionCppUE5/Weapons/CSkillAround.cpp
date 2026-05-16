#include "Weapons/CSkillAround.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Utilities/CLog.h"

ACSkillAround::ACSkillAround()
{
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(DefaultSceneRoot);

	ParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystem"));
	ParticleSystem->SetupAttachment(Sphere);
}

bool ACSkillAround::Around_IsCharacterRealRiding() const
{
	if (false == IsValid(Character))
	{
		return false;
	}

	// State 가 이미 같은 UCStateComponent 를 가리키면 재탐색 생략(레거시 SetCharacterState 플로우 호환).
	UCStateComponent* StateComp = Cast<UCStateComponent>(State.Get());
	if (false == IsValid(StateComp))
	{
		StateComp = Character->FindComponentByClass<UCStateComponent>();
	}

	if (false == IsValid(StateComp))
	{
		// 매 프레임 호출 가능 — 조용히 false.
		return false;
	}

	return StateComp->IsRealRiding();
}

void ACSkillAround::Around_RefreshStateFromCharacterComponent()
{
	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillAround] Refresh State: Character 없음 — 컨텍스트=%s Owner=%s"),
			*GetNameSafe(this),
			*GetNameSafe(GetOwner())));
		State = nullptr;
		return;
	}

	UCStateComponent* Resolved = Character->FindComponentByClass<UCStateComponent>();
	State = Resolved;
	if (false == IsValid(Resolved))
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillAround] Refresh State: UCStateComponent 없음 — Character=%s 무기 액터=%s"),
			*GetNameSafe(Character),
			*GetNameSafe(this)));
	}
}
