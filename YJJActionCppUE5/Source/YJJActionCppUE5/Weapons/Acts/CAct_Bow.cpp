#include "Weapons/ACts/CAct_Bow.h"
#include "Global.h"
#include "Weapons/CEquipment.h"
#include "Weapons/Attachments/CAttachment_Bow.h"
#include "Weapons/AddOns/CArrow.h"
#include "GameFramework/Character.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/CStateComponent.h"

UCAct_Bow::UCAct_Bow()
{
	ArrowClass = ACArrow::StaticClass();
}

void UCAct_Bow::BeginPlay(
	TWeakObjectPtr<ACCommonCharacter> InOwner,
	TWeakObjectPtr<ACAttachment> InAttachment,
	TWeakObjectPtr<UCEquipment> InEquipment, 
	const TArray<FActData>& InDoActionData,
	const TArray<FHitData>& InHitData)
{
	Super::BeginPlay(InOwner, InAttachment, InEquipment, InDoActionData, InHitData);

	SkeletalMesh = YJJHelpers::GetComponent<USkeletalMeshComponent>(InAttachment.Get());
	PoseableMesh = YJJHelpers::GetComponent<UPoseableMeshComponent>(InAttachment.Get());


	const TWeakObjectPtr<ACAttachment_Bow> bow = Cast<ACAttachment_Bow>(InAttachment);
	Bending = MakeShared<float>(*bow->GetBend());

	OriginLocation = PoseableMesh->GetBoneLocationByName("bow_string_mid", EBoneSpaces::ComponentSpace);

	bEquipped = MakeShared<bool>(*InEquipment->GetEquipped());
}

void UCAct_Bow::OnBeginEquip()
{
	Super::OnBeginEquip();

	Owner->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	CreateArrow();
}

void UCAct_Bow::OnUnequip()
{
	Super::OnUnequip();

	*Bending = 0;
	Owner->GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	PoseableMesh->SetBoneLocationByName("bow_string_mid", OriginLocation, EBoneSpaces::ComponentSpace);

	for (int32 i = Arrows.Num() - 1; i >= 0; i--)
	{
		if (IsValid(Arrows[i]->GetAttachParentActor()))
			Arrows[i]->Destroy();
	}
}

void UCAct_Bow::Act()
{
	CheckFalse(StateComp->IsIdleMode());
	CheckFalse(StateComp->IsSkillMode());

	Super::Act();

	ActDatas[0].Act(Owner);
}

void UCAct_Bow::Begin_Act()
{
	Super::Begin_Act();

	bAttachedString = false;

	*Bending = 0.0f;
	PoseableMesh->SetBoneLocationByName("bow_string_mid", OriginLocation, EBoneSpaces::ComponentSpace);

	CheckNull(ArrowClass);

	const TWeakObjectPtr<ACArrow> arrow = GetAttachedArrow();
	arrow->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));

	arrow->OnArrowHit.AddDynamic(this, &UCAct_Bow::OnArrowHit);
	arrow->OnArrowEndPlay.AddDynamic(this, &UCAct_Bow::OnArrowEndPlay);

	const FVector forward = FQuat(Owner->GetControlRotation()).GetForwardVector();
	arrow->Shoot(forward);
}

void UCAct_Bow::End_Act()
{
	Super::End_Act();

	CreateArrow();
}

void UCAct_Bow::Tick(float InDeltaTime)
{
	Super::Tick(InDeltaTime);


	PoseableMesh->CopyPoseFromSkeletalComponent(SkeletalMesh);

	bool bCheck = true;
	bCheck &= (*bEquipped == true);
	bCheck &= (bBeginAct == false);
	bCheck &= (bAttachedString == true);

	CheckFalse(bCheck);

	const FVector handLocation = Owner->GetMesh()->GetSocketLocation("Hand_Bow_Right");
	PoseableMesh->SetBoneLocationByName("bow_string_mid", handLocation, EBoneSpaces::WorldSpace);
}

void UCAct_Bow::End_BowString()
{
	*Bending = 100.0f;
	bAttachedString = true;
}

void UCAct_Bow::CreateArrow()
{
	// 레벨 이동, 게임 종료
	if (World->bIsTearingDown == true)
		return;

	const FTransform transform;
	const TWeakObjectPtr<ACArrow> arrow = World->SpawnActorDeferred<ACArrow>(
		ArrowClass,
		transform,
		NULL,
		NULL,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	CheckNull(arrow);

	arrow->AddIgnoreActor(Owner);

	const FAttachmentTransformRules rule = 
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true);
	arrow->AttachToComponent(Owner->GetMesh(), rule, "Hand_Bow_Right_Arrow");

	Arrows.Add(arrow.Get());
	UGameplayStatics::FinishSpawningActor(arrow.Get(), transform);
}

TObjectPtr<ACArrow> UCAct_Bow::GetAttachedArrow()
{
	for (int i = Arrows.Num() - 1; i >= 0; i--)
	{
		if (!!Arrows[i]->GetAttachParentActor())
			return Arrows[i];
	}

	return nullptr;
}

void UCAct_Bow::OnArrowHit(AActor* InCauser, ACCommonCharacter* InOtherCharacter)
{
	CheckFalse(HitDatas.Num() > 0);

	HitDatas[0].SendDamage(Owner, InCauser, InOtherCharacter);
}

void UCAct_Bow::OnArrowEndPlay(ACArrow* InDestroyer)
{
	Arrows.Remove(InDestroyer);
}