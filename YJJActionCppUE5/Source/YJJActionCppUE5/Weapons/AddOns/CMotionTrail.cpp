#include "Weapons/AddOns/CMotionTrail.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"

ACMotionTrail::ACMotionTrail()
{
	YJJHelpers::CreateComponent<UPoseableMeshComponent>(this, &Mesh, "Mesh");
}

void ACMotionTrail::BeginPlay()
{
	Super::BeginPlay();

	TObjectPtr<UMaterialInstanceConstant> material;
	YJJHelpers::GetAssetDynamic<UMaterialInstanceConstant>(
		&material, "/Script/Engine.MaterialInstanceConstant'/Game/Materials/MotionTrail/M_MotionTrail_Inst.M_MotionTrail_Inst'");

	Material = UMaterialInstanceDynamic::Create(material, this);
	Material->SetVectorParameterValue("Color", Color);
	Material->SetScalarParameterValue("Exponent", Exponent);

	Owner = Cast<ACCommonCharacter>(GetOwner());

	Mesh->SetVisibility(false);
	Mesh->SetSkinnedAsset(Owner->GetMesh()->GetSkeletalMeshAsset());
	Mesh->CopyPoseFromSkeletalComponent(Owner->GetMesh());
	Mesh->SetRelativeScale3D(Scale);

	MaterialSlotCount = Owner->GetMesh()->GetSkinnedAsset()->GetMaterials().Num();

	for (int32 i = 0; i < MaterialSlotCount; i++)
		Mesh->SetMaterial(i, Material);

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&ACMotionTrail::OnCaptureInterval,
		Interval,
		true,
		StartDelay);
}

void ACMotionTrail::OnCaptureInterval()
{
	if (!Mesh->IsVisible())
		Mesh->ToggleVisibility();

	const float height = Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	SetActorLocation(
		Owner->GetActorLocation() -
		FVector(ScaleAmount.X, ScaleAmount.Y, height - ScaleAmount.Z));

	SetActorRotation(
		Owner->GetActorRotation() +
		FRotator(0, -90, 0));

	Mesh->CopyPoseFromSkeletalComponent(Owner->GetMesh());

	OriginalExponent = Exponent;

	if (DisappearFlag)
	{
		GetWorld()->GetTimerManager().SetTimer(
			TimerDisappearHandle,
			this,
			&ACMotionTrail::OnDisappearTick,
			DisappearInterval,
			true,
			DisappearStartDelay);
	}
}

void ACMotionTrail::OnDisappearTick()
{
	Exponent -= DisappearExponent;

	Material->SetScalarParameterValue("Exponent", Exponent);

	for (int32 i = 0; i < MaterialSlotCount; i++)
		Mesh->SetMaterial(i, Material);

	if (Exponent <= 0.0f)
	{
		Exponent = OriginalExponent;
		GetWorld()->GetTimerManager().ClearTimer(TimerDisappearHandle);
	}
}

void ACMotionTrail::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(TimerDisappearHandle);
}
