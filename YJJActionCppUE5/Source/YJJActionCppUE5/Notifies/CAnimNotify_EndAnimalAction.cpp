#include "Notifies/CAnimNotify_EndAnimalAction.h"

#include "Characters/Animals/CAnimal.h"
#include "Characters/Animals/CAnimalWeapon.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
	static ACAnimalWeapon* ResolveAnimalWeaponLikeCollisionNotify(ACAnimal* OwningAnimal)
	{
		// UCAnimNotifyState_AnimalCollision 과 동일한 슬롯 우선순위(일반 무기 슬롯 → 드래곤 무기).
		if (nullptr == OwningAnimal)
		{
			return nullptr;
		}

		AActor* weaponActorLocal = OwningAnimal->AnimalWeapon.Get();
		if (nullptr != weaponActorLocal)
		{
			ACAnimalWeapon* const typedLocal = Cast<ACAnimalWeapon>(weaponActorLocal);
			if (IsValid(typedLocal))
			{
				return typedLocal;
			}
		}

		ACDragon* const dragonLocal = Cast<ACDragon>(OwningAnimal);

		if (false == IsValid(dragonLocal))
		{
			return nullptr;
		}

		weaponActorLocal = dragonLocal->DragonWeapon.Get();

		return Cast<ACAnimalWeapon>(weaponActorLocal);
	}
} // namespace

FString UCAnimNotify_EndAnimalAction::GetNotifyName_Implementation() const
{
	return TEXT("End_AnimalAction");
}

void UCAnimNotify_EndAnimalAction::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (false == IsValid(MeshComp))
	{
		return;
	}

	AActor* const meshOwner = MeshComp->GetOwner();

	ACAnimal* const animalScratch = Cast<ACAnimal>(meshOwner);

	if (false == IsValid(animalScratch))
	{
		// 비동물 소유 스켈메시 — 레거시로는 발생하지 않았을 것이지만 공용 에셋 꽂기 시 무시한다(노티 호출량이 많을 수 있어 로그 없음).
		return;
	}

	ACAnimalWeapon* const weaponScratch = ResolveAnimalWeaponLikeCollisionNotify(animalScratch);

	if (false == IsValid(weaponScratch))
	{
		// 무기 미스폰 또는 블프 전용 무기(ACAnimalWeapon 아님) — 애니 끝 프레임의 한두 번 집합 정도여서 조용히 둠.
		return;
	}

	weaponScratch->End_DoAction();
}
