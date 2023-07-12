#include "Notifies/CAnimNotifyState_MotionTrail.h"
#include "Characters/CCommonCharacter.h"
#include "Global.h"

FString UCAnimNotifyState_MotionTrail::GetNotifyName_Implementation() const
{
	return "MotionTrail";
}

void UCAnimNotifyState_MotionTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	const TWeakObjectPtr<ACCommonCharacter> owner = Cast<ACCommonCharacter>(MeshComp->GetOwner());
	CheckNull(owner);

	MotionTrail = YJJHelpers::PlayMotionTrail(MotionTrailClass, owner);
}

void UCAnimNotifyState_MotionTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (MotionTrail.IsValid())
		MotionTrail->Destroy();
}