#include "Notifies/CAnimNotify_Move.h"
#include "Components/CMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

FString UCAnimNotify_Move::GetNotifyName_Implementation() const
{
	return TEXT("Move");
}

void UCAnimNotify_Move::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (false == IsValid(MeshComp))
		return;

	AActor* meshOwnerActor = MeshComp->GetOwner();
	if (false == IsValid(meshOwnerActor))
		return;

	UCMovementComponent* movementComponent =
		meshOwnerActor->FindComponentByClass<UCMovementComponent>();
	if (false == IsValid(movementComponent))
	{
		// 애니를 탄 오브젝트가 캐릭터 계열이 아니거나 네이티브 이동 컴포넌트 미부착 — 공용 노티라 로그 없이 무시한다.
		return;
	}

	movementComponent->Move();
}
