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
	SkeletalMesh->SetSkeletalMeshAsset(mesh.Get());
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

	PoseableMesh->SetSkinnedAsset(SkeletalMesh->GetSkeletalMeshAsset());
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

TSharedPtr<float> ACAttachment_Bow::GetBend()
{
	if (false == IsValid(SkeletalMesh))
	{
		CLog::Log(TEXT("[Bow] GetBend — SkeletalMesh 없음"));
		return MakeShared<float>(0.0f);
	}

	// Spawn 직후·비표시 메시 등에서 AnimInstance가 아직 없을 수 있어 한 번 초기화를 시도한다.
	USkeletalMeshComponent* mesh = SkeletalMesh;
	UCAnimInstance_Bow* bowAnim = Cast<UCAnimInstance_Bow>(mesh->GetAnimInstance());
	if (false == IsValid(bowAnim))
	{
		mesh->InitAnim(false);
		bowAnim = Cast<UCAnimInstance_Bow>(mesh->GetAnimInstance());
	}

	if (false == IsValid(bowAnim))
	{
		CLog::Log(FString::Printf(
			TEXT("[Bow] GetBend — UCAnimInstance_Bow 없음 (AnimClass=%s)"),
			IsValid(mesh->GetAnimClass()) ? *mesh->GetAnimClass()->GetName() : TEXT("(null)")));
		return MakeShared<float>(0.0f);
	}

	return bowAnim->GetBend();
}