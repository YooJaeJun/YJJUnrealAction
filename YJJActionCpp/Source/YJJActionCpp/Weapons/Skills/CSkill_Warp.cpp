#include "Weapons/Skills/CSkill_Warp.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/PlayerController.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CCamComponent.h"

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

	CheckNull(Controller);
	CheckNull(CameraActor);

	CameraActor->SetActorLocation(Owner->GetActorLocation() + CameraRelativeLocation);
}

void UCSkill_Warp::Pressed()
{
	CheckNull(Controller);
	CheckTrue(StateComp->IsSkillMode());

	Super::Pressed();

	StateComp->OnSkillMode();
	CamComp->EnableTopViewCamera();
	Controller->SetViewTargetWithBlend(CameraActor.Get(), BlendIn);
}

void UCSkill_Warp::Released()
{
	CheckNull(Controller);
	CheckFalse(StateComp->IsSkillMode());

	Super::Released();

	StateComp->OffSkillMode();
	CamComp->DisableTopViewCamera();
	Controller->SetViewTargetWithBlend(Owner.Get(), BlendIn);
}