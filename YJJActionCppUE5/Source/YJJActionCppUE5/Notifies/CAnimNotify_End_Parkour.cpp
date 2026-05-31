#include "Notifies/CAnimNotify_End_Parkour.h"

#include "Global.h"
#include "Components/CParkourComponent.h"

FString UCAnimNotify_End_Parkour::GetNotifyName_Implementation() const
{
	return TEXT("End_Parkour");
}

void UCAnimNotify_End_Parkour::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (nullptr == MeshComp)
	{
		CLog::Log(TEXT("[End_Parkour] MeshComp 가 null 입니다."));
		return;
	}

	AActor* const OwnerActor = MeshComp->GetOwner();
	if (nullptr == OwnerActor)
	{
		CLog::Log(TEXT("[End_Parkour] MeshComp 의 Owner 없음."));
		return;
	}

	TObjectPtr<UCParkourComponent> const parkourResolved = YJJHelpers::GetComponent<UCParkourComponent>(OwnerActor);

	// 파쿠르 없는 캐릭터·공유 애님에서는 블프 IsValid 분기처럼 그냥 생략(스팸 방지 로그 없음).
	if (false == IsValid(parkourResolved))
		return;

	parkourResolved->End_DoParkour();
}
