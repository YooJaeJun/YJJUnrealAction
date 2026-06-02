#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAttachment.h"
#include "CAttachment_Melee.generated.h"

/** 레거시 CBP_Attachment_Fist/Sword/Hammer/Dual — 충돌 Shape 는 BP 자식 또는 에디터에서 구성. */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACAttachment_Fist : public ACAttachment
{
	GENERATED_BODY()

public:
	ACAttachment_Fist();
};

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACAttachment_Sword : public ACAttachment
{
	GENERATED_BODY()

public:
	ACAttachment_Sword();
};

class USkeletalMeshComponent;

/** 레거시 CBP_Attachment_Hammer EventGraph — BeginPlay/Unequip 시 SkeletalMesh 숨김, BeginEquip 시 표시 + Hand_Hammer 부착. */
UCLASS(Blueprintable, meta = (DisplayName = "CAttachmentHammer"))
class YJJACTIONCPPUE5_API ACAttachment_Hammer : public ACAttachment
{
	GENERATED_BODY()

public:
	ACAttachment_Hammer();

protected:
	virtual void BeginPlay() override;

	virtual void OnBeginEquip_Implementation() override;
	virtual void OnUnequip_Implementation() override;

private:
	USkeletalMeshComponent* ResolveSkeletalMesh();
	void SetSkeletalMeshVisible(const bool bVisible);

	UPROPERTY(EditDefaultsOnly, Category = "Attach")
	FName HandEquipSocketName = TEXT("Hand_Hammer");

	TWeakObjectPtr<USkeletalMeshComponent> CachedSkeletalMesh;
};

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACAttachment_Dual : public ACAttachment
{
	GENERATED_BODY()

public:
	ACAttachment_Dual();
};
