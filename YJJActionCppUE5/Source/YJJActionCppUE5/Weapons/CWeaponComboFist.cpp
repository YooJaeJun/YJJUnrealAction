#include "Weapons/CWeaponCombo.h"

#include "Characters/CCommonCharacter.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ShapeComponent.h"
#include "Utilities/CLog.h"

ACWeaponComboFist::ACWeaponComboFist()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACWeaponComboFist::BeginPlay()
{
	Super::BeginPlay();

	ComboFist_AttachCollisionShapesToCharacterMesh();
}

void ACWeaponComboFist::ComboFist_AttachCollisionShapesToCharacterMesh()
{
	if (false == IsValid(Character))
	{
		return;
	}

	const ACharacter* const CharActor = Cast<ACharacter>(Character);
	if (false == IsValid(CharActor))
	{
		return;
	}

	USkeletalMeshComponent* const CharacterMesh = CharActor->GetMesh();
	if (false == IsValid(CharacterMesh))
	{
		CLog::Log(FString::Printf(
			TEXT("[WeaponComboFist] Character Mesh 없음 — %s Character=%s"),
			*GetNameSafe(this),
			*GetNameSafe(Character)));
		return;
	}

	// 레거시 Combo_Fist BP: Collisions ForEach → Attach, SocketName = Shape 컴포넌트 Object Name.
	static const FAttachmentTransformRules AttachRules(
		EAttachmentRule::KeepRelative,
		EAttachmentRule::KeepRelative,
		EAttachmentRule::KeepRelative,
		true);

	const int32 shapeCount = ComboCollisionShapes.Num();
	for (int32 shapeIndex = 0; shapeIndex < shapeCount; ++shapeIndex)
	{
		UShapeComponent* const Shape = ComboCollisionShapes[shapeIndex].Get();
		if (false == IsValid(Shape))
		{
			continue;
		}

		const FName SocketName = Shape->GetFName();
		Shape->AttachToComponent(CharacterMesh, AttachRules, SocketName);
	}
}

