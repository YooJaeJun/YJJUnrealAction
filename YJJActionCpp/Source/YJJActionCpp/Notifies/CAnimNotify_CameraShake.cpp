#include "Notifies/CAnimNotify_CameraShake.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Camera/CameraShake.h"

FString UCAnimNotify_CameraShake::GetNotifyName_Implementation() const
{
	return "CameraShake";
}

void UCAnimNotify_CameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	CheckNull(MeshComp);
	CheckNull(MeshComp->GetOwner());

	ACCommonCharacter* character = Cast<ACCommonCharacter>(MeshComp->GetOwner());
	CheckNull(character);

	APlayerController* controller = Cast<APlayerController>(character->MyCurController);
	CheckNull(controller);

	controller->PlayerCameraManager->StartCameraShake(CameraShakeClass);
}
