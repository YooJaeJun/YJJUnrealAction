#include "Notifies/CAnimNotifyState_AnimalCollision.h"

#include "Characters/Animals/CAnimal.h"
#include "Characters/Animals/CAnimalWeapon.h"
#include "Characters/Animals/Dragon/CDragon.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

namespace
{
	static ACAnimalWeapon* ResolveAnimalWeaponForCollision(ACAnimal* OwningAnimal)
	{
		// 순서 동일 유지 — 일반 Animal 은 AnimalWeapon 슬롯이 주 경로(ACDragon 은 빈 칸 가능).
		if (nullptr == OwningAnimal)
		{
			return nullptr;
		}

		AActor* weaponActorLocal = OwningAnimal->AnimalWeapon.Get();
		if (nullptr != weaponActorLocal)
		{
			ACAnimalWeapon* const typedLocal = Cast<ACAnimalWeapon>(weaponActorLocal);
			if (IsValid(typedLocal))
				return typedLocal;
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

FString UCAnimNotifyState_AnimalCollision::GetNotifyName_Implementation() const
{
	return TEXT("Animal_Collision");
}

void UCAnimNotifyState_AnimalCollision::NotifyBegin(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float TotalDuration,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (false == IsValid(MeshComp))
	{
		return;
	}

	AActor* const meshOwner = MeshComp->GetOwner();

	ACAnimal* const animalScratch = Cast<ACAnimal>(meshOwner);

	if (false == IsValid(animalScratch))
	{
		// 메시 소유자가 사람/프랍 등 비동물 — 이 노티를 공용 몽타주에 꽂은 경우 종종 발생, 틱·노티 빈발로 로그 없음.
		return;
	}

	ACAnimalWeapon* const weaponScratch = ResolveAnimalWeaponForCollision(animalScratch);

	if (false == IsValid(weaponScratch))
	{
		// 스폰 직후·블프 무기 클래스 불일치 — 몽타주만 재생된 프레임에 한 번씩이라 조용히 둠.
		return;
	}

	weaponScratch->OnCollisions();
}

void UCAnimNotifyState_AnimalCollision::NotifyEnd(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (false == IsValid(MeshComp))
	{
		return;
	}

	AActor* const meshOwner = MeshComp->GetOwner();

	ACAnimal* const animalScratch = Cast<ACAnimal>(meshOwner);

	if (false == IsValid(animalScratch))
	{
		return;
	}

	ACAnimalWeapon* const weaponScratch = ResolveAnimalWeaponForCollision(animalScratch);

	if (false == IsValid(weaponScratch))
	{
		return;
	}

	weaponScratch->OffCollisions();
}
