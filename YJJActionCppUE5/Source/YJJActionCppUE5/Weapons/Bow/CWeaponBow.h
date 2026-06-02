#pragma once

#include "CoreMinimal.h"
#include "Weapons/CSkillWeapon.h"
#include "CWeaponBow.generated.h"

class UPoseableMeshComponent;
class ACSkillArrow;
class ACSkillAim;

/**
 * 레거시 `/Game/Weapons/Bow/Weapon_Bow` — `/Game/Weapons/Weapon` (ACWeapon) 계승 + 활 줄·연출용 변수.
 * PoseableMesh 서브오브젝트명은 에디터 `PoseableMesh` 와 동일하게 두고, C++ 멤버만 BowPoseableMesh 로 두어 식별 충돌을 피한다.
 */
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACWeaponBow : public ACWeapon
{
	GENERATED_BODY()

public:
	ACWeaponBow();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void Equip_Implementation() override;

	virtual void Pressed_Implementation() override;

	virtual void Released_Implementation() override;

	virtual void Begin_Equip_Implementation(bool bMainOrSubWeapon) override;

	virtual void End_Equip_Implementation(bool bMainOrSubWeapon) override;

	virtual void Begin_Unequip_Implementation(bool bMainOrSubWeapon) override;

	virtual void End_Unequip_Implementation(bool bMainOrSubWeapon) override;

	virtual void Begin_DoAction_Implementation(CEAttackType InAttackType) override;

	virtual void End_DoAction(CEAttackType InAttackType) override;

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Sockets")
	FName HolsterSocketName = FName(TEXT("Holster_Bow"));

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Sockets")
	FName HandLeftSocketName = FName(TEXT("Hand_Bow_Left"));

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Sockets")
	FName PullHandSocketName = FName(TEXT("Hand_Bow_Right"));

	/** 새 화살을 캐릭터 메시에 고정할 소켓(레거시 CAct 활: Hand_Bow_Right_Arrow). */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Sockets")
	FName ArrowAttachSocketName = FName(TEXT("Hand_Bow_Right_Arrow"));

	UPROPERTY(EditDefaultsOnly, Category = "Bow|Arrow")
	TSubclassOf<ACSkillArrow> SkillArrowSpawnClass;

	/** 레거시 End_Equip 에서 SpawnActor Skill_Aim_C — 디폴트는 BP 클래스 디폴트에서 채운다. */
	UPROPERTY(EditDefaultsOnly, Category = "Bow|Aim")
	TSubclassOf<ACSkillAim> AimSpawnClass;

	/** 레거시 BP `/Game/Weapons/Bow/Weapon_Bow:CreateArrow` 와 이름 정합성. */
	UFUNCTION(BlueprintCallable, Category = "Bow|Arrow")
	void CreateArrow();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
	TObjectPtr<UPoseableMeshComponent> BowPoseableMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (BlueprintPrivate = "true", MultiLine = "true"))
	TObjectPtr<ACSkillArrow> Arrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (BlueprintPrivate = "true", MultiLine = "true"))
	TArray<TObjectPtr<ACSkillArrow>> Arrows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (BlueprintPrivate = "true", MultiLine = "true"))
	TObjectPtr<ACSkillAim> Aim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (MultiLine = "true"))
	FName StringName = FName(TEXT("bow_string_mid"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (MultiLine = "true"))
	FVector OriginLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (MultiLine = "true"))
	bool AttachBowString = true;

private:
	/** 레거시 BP: 일반 장착·손 소켓은 KeepRelative 균일, Unequip 시 Holster 만 Location/Rotation KeepRelative·Scale KeepWorld */
	void Bow_AttachWeaponToCharacterSocket(const FName& SocketName, bool bUnequipHolsterRules = false);

	void RefreshBowStringOrigin_ComponentSpace();

	void Bow_SetCharacterMeshCollision(ECollisionEnabled::Type NewCollision);

	void Bow_ResetStringBoneToOrigin_ComponentSpace();

	void Bow_DestroyAimActorIfValid();

	void Bow_SpawnAimActorOwnedByBow();

	void Bow_DestroyArrowsAttachedToCharacter();

	UFUNCTION()
	void WeaponBow_OnArrowDestroyed(UObject* InArrow);
};
