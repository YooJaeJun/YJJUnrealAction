#include "Weapons/Attachments/CAttachment_Bow.h"
#include "Global.h"
#include "Weapons/AnimInstances/CAnimInstance_Bow.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PoseableMeshComponent.h"


ACAttachment_Bow::ACAttachment_Bow()
{
	YJJHelpers::CreateComponent<USkeletalMeshComponent>(this, &SkeletalMesh, "SkeletalMesh", Root);
	YJJHelpers::CreateComponent<UPoseableMeshComponent>(this, &PoseableMesh, "PoseableMesh", Root);

	TObjectPtr<USkeletalMesh> mesh;
	YJJHelpers::GetAsset<USkeletalMesh>(&mesh, "/Script/Engine.SkeletalMesh'/Game/Assets/Character/Weapons/ElvenBow/SK_ElvenBow.SK_ElvenBow'");
	SkeletalMesh->SetSkeletalMesh(mesh.Get());
	SkeletalMesh->SetCollisionProfileName("NoCollision");

	TSubclassOf<UCAnimInstance_Bow> animInstance;
	YJJHelpers::GetClass<UCAnimInstance_Bow>(&animInstance, "/Script/Engine.AnimBlueprint'/Game/Weapons/Bow/CABP_Bow.CABP_Bow_C'");
	SkeletalMesh->SetAnimInstanceClass(animInstance);
}

void ACAttachment_Bow::BeginPlay()
{
	Super::BeginPlay();

	AttachTo("Holster_Bow");

	SkeletalMesh->SetVisibility(false);

	PoseableMesh->SetSkinnedAsset(SkeletalMesh->SkeletalMesh.Get());
	PoseableMesh->CopyPoseFromSkeletalComponent(SkeletalMesh);
}

void ACAttachment_Bow::OnBeginEquip_Implementation()
{
	Super::OnBeginEquip_Implementation();

	AttachTo("Hand_Bow_Left");

	const TWeakObjectPtr<APlayerController> controller = Owner->GetController<APlayerController>();
	if (controller.IsValid())
	{
		OriginViewPitchRange.X = controller->PlayerCameraManager->ViewPitchMin;
		OriginViewPitchRange.Y = controller->PlayerCameraManager->ViewPitchMax;

		controller->PlayerCameraManager->ViewPitchMin = ViewPitchRange.X;
		controller->PlayerCameraManager->ViewPitchMax = ViewPitchRange.Y;
	}
}

void ACAttachment_Bow::OnUnequip_Implementation()
{
	Super::OnUnequip_Implementation();

	AttachTo("Holster_Bow");

	const TWeakObjectPtr<APlayerController> controller = Owner->GetController<APlayerController>();
	if (controller.IsValid())
	{
		controller->PlayerCameraManager->ViewPitchMin = OriginViewPitchRange.X;
		controller->PlayerCameraManager->ViewPitchMax = OriginViewPitchRange.Y;
	}
}

TSharedPtr<float> ACAttachment_Bow::GetBend() const
{
	return Cast<UCAnimInstance_Bow>(SkeletalMesh->GetAnimInstance())->GetBend();
}