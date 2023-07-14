#include "Weapons/Skills/CSkill_Aiming.h"
#include "Global.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Components/CCamComponent.h"
#include "Components/CStateComponent.h"
#include "Weapons/Attachments/CAttachment_Bow.h"

UCSkill_Aiming::UCSkill_Aiming()
{
	YJJHelpers::GetAsset<UCurveVector>(&Curve, "CurveVector'/Game/Weapons/Bow/CCurve_Aiming.CCurve_Aiming'");
}

void UCSkill_Aiming::BeginPlay(TWeakObjectPtr<ACCommonCharacter> InOwner, ACAttachment* InAttachment, UCAct* InAct)
{
	Super::BeginPlay(InOwner, InAttachment, InAct);

	SpringArm = YJJHelpers::GetComponent<USpringArmComponent>(InOwner.Get());
	Camera = YJJHelpers::GetComponent<UCameraComponent>(InOwner.Get());

	FOnTimelineVector timeline;
	timeline.BindUFunction(this, "OnAiming");

	Timeline.AddInterpVector(Curve, timeline);
	Timeline.SetPlayRate(AimingSpeed);

	const TWeakObjectPtr<ACAttachment_Bow> bow = Cast<ACAttachment_Bow>(InAttachment);
	CheckNull(bow);

	Bend = bow->GetBend();
}

void UCSkill_Aiming::Tick_Implementation(float InDeltaTime)
{
	Super::Tick_Implementation(InDeltaTime);

	Timeline.TickTimeline(InDeltaTime);
}

void UCSkill_Aiming::Pressed()
{
	CheckTrue(StateComp->IsSkillMode());
	CheckNull(SpringArm);
	CheckNull(Camera);

	Super::Pressed();

	StateComp->OnSkillMode();

	OriginData.TargetArmLength = SpringArm->TargetArmLength;
	OriginData.SocketOffset = SpringArm->SocketOffset;
	OriginData.bEnableCameraLag = SpringArm->bEnableCameraLag;
	OriginData.CameraLocation = Camera->GetRelativeLocation();

	SpringArm->TargetArmLength = AimData.TargetArmLength;
	SpringArm->SocketOffset = AimData.SocketOffset;
	SpringArm->bEnableCameraLag = AimData.bEnableCameraLag;
	Camera->SetRelativeLocation(AimData.CameraLocation);

	Timeline.PlayFromStart();
}

void UCSkill_Aiming::Released()
{
	CheckFalse(StateComp->IsSkillMode());
	CheckNull(SpringArm);
	CheckNull(Camera);

	Super::Released();

	StateComp->OffSkillMode();

	SpringArm->TargetArmLength = OriginData.TargetArmLength;
	SpringArm->SocketOffset = OriginData.SocketOffset;
	SpringArm->bEnableCameraLag = OriginData.bEnableCameraLag;
	Camera->SetRelativeLocation(OriginData.CameraLocation);

	Timeline.ReverseFromEnd();
}

void UCSkill_Aiming::OnAiming(FVector Output)
{
	Camera->FieldOfView = Output.X;

	CheckNull(Bend);
	*Bend = Output.Y;
}