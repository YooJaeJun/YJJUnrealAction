#include "Notifies/CAnimNotifyState_Zoom.h"
#include "Global.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Components/CCamComponent.h"

FString UCAnimNotifyState_Zoom::GetNotifyName_Implementation() const
{
	return "Zoom";
}

void UCAnimNotifyState_Zoom::NotifyBegin(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	float TotalDuration, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<ACPlayableCharacter> player = Cast<ACPlayableCharacter>(MeshComp->GetOwner());
	CheckNull(player);

	const TWeakObjectPtr<UCCamComponent> camComp = YJJHelpers::GetComponent<UCCamComponent>(player.Get());
	CheckNull(camComp);

	OriginZooming = camComp->GetZooming();
	camComp->SetZooming(SkillZooming);
}

void UCAnimNotifyState_Zoom::NotifyEnd(
	USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	const TWeakObjectPtr<ACPlayableCharacter> player = Cast<ACPlayableCharacter>(MeshComp->GetOwner());
	CheckNull(player);

	const TWeakObjectPtr<UCCamComponent> camComp = YJJHelpers::GetComponent<UCCamComponent>(player.Get());
	CheckNull(camComp);

	camComp->SetZooming(OriginZooming);
	OriginZooming = 0.0f;
}
