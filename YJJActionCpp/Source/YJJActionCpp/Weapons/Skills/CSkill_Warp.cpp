#include "Weapons/Skills/CSkill_Warp.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"

UCSkill_Warp::UCSkill_Warp()
{
	CameraActorClass = ACameraActor::StaticClass();
}

void UCSkill_Warp::BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner, ACAttachment* InAttachment, UCAct* InAct)
{
	Super::BeginPlay(InOwner, InAttachment, InAct);

	Controller = InOwner->GetController<APlayerController>();

	CameraActor = InOwner->GetWorld()->SpawnActor<ACameraActor>(CameraActorClass.Get());
	CameraActor->SetActorRotation(FRotator(-90, 0, 0));

	const TWeakObjectPtr<UCameraComponent> camera =
		YJJHelpers::GetComponent<UCameraComponent>(CameraActor.Get());
	camera->ProjectionMode = ProjectionMode;
	camera->OrthoWidth = OrthoWidth;
	camera->FieldOfView = FieldOfView;
}

void UCSkill_Warp::Tick_Implementation(float InDeltaTime)
{
	Super::Tick_Implementation(InDeltaTime);
}

void UCSkill_Warp::Pressed()
{
	CheckNull(Controller);
	CheckTrue(StateComp->IsSkillMode());

	Super::Pressed();

	StateComp->OnSkillMode();
	//MovementComp->EnableTopViewCamera();

}

void UCSkill_Warp::Released()
{
	Super::Released();
}