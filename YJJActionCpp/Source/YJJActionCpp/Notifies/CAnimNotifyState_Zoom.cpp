#include "Notifies/CAnimNotifyState_Zoom.h"
#include "Global.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Components/CCameraComponent.h"

FString UCAnimNotifyState_Zoom::GetNotifyName_Implementation() const
{
	return "Zoom";
}

void UCAnimNotifyState_Zoom::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	const TWeakObjectPtr<ACPlayableCharacter> player = Cast<ACPlayableCharacter>(MeshComp->GetOwner());
	CheckNull(player);

	const TWeakObjectPtr<UCCameraComponent> zoomComp = YJJHelpers::GetComponent<UCCameraComponent>(player.Get());
	CheckNull(zoomComp);

	OriginZooming = zoomComp->GetZooming();
	zoomComp->SetZooming(SkillZooming);
}

void UCAnimNotifyState_Zoom::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	const TWeakObjectPtr<ACPlayableCharacter> player = Cast<ACPlayableCharacter>(MeshComp->GetOwner());
	CheckNull(player);

	const TWeakObjectPtr<UCCameraComponent> zoomComp = YJJHelpers::GetComponent<UCCameraComponent>(player.Get());
	CheckNull(zoomComp);

	zoomComp->SetZooming(OriginZooming);
	OriginZooming = 0.0f;
}
