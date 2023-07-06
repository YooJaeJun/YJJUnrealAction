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

	UMaterialInstanceConstant* material;
	YJJHelpers::GetAssetDynamic<UMaterialInstanceConstant>(
		&material, "MaterialInstanceConstant'/Game/Materials/MotionTrail/M_MotionTrail_Inst.M_MotionTrail_Inst'");

	Material = UMaterialInstanceDynamic::Create(material, this);
	Material->SetVectorParameterValue("Color", Color);
	Material->SetScalarParameterValue("Exponent", Exponent);

	Owner = Cast<ACCommonCharacter>(GetOwner());

	Mesh->SetVisibility(false);
	Mesh->SetSkeletalMesh(Owner->GetMesh()->SkeletalMesh);
	Mesh->CopyPoseFromSkeletalComponent(Owner->GetMesh());
	Mesh->SetRelativeScale3D(Scale);

	const int32 size = Owner->GetMesh()->SkeletalMesh->Materials.Num();

	for (int32 i = 0; i < size; i++)
		Mesh->SetMaterial(i, Material.Get());


	FTimerDelegate timerDelegate;
	timerDelegate.BindLambda([=]()
		{
			if (Mesh->IsVisible() == false)
				Mesh->ToggleVisibility();

			const float height = Owner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			SetActorLocation(
				Owner->GetActorLocation() - 
				FVector(ScaleAmount.X, ScaleAmount.Y, height - ScaleAmount.Z));

			SetActorRotation(
				Owner->GetActorRotation() + 
				FRotator(0, -90, 0));

			Mesh->CopyPoseFromSkeletalComponent(Owner->GetMesh());
		});

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle, 
		timerDelegate, 
		Interval, 
		true, 
		StartDelay);
}

void ACMotionTrail::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}