#pragma once

#include "CoreMinimal.h"
#include "Weapons/CSkillWeapon.h"
#include "CSkillAim.generated.h"

class USpringArmComponent;
class UCameraComponent;
class AController;
class UUserWidget;

// BP `Skill_Aim` — Skill_Weapon BeginPlay 이후 조준용 카메라 참조·크로스헤어(숨김) 준비.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACSkillAim : public ACSkillWeapon
{
	GENERATED_BODY()

public:
	ACSkillAim();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	TObjectPtr<AController> AimController;

	UPROPERTY(BlueprintReadOnly, Category = "Aim")
	TObjectPtr<UUserWidget> CrossHair;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Aim")
	TSubclassOf<UUserWidget> CrossHairWidgetClass;

public:
	/** 레거시 BP Skill_Aim::Pressed — 블루프린트에서 조준 시작 로직 오버라이드. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Aim")
	void Pressed();

	/** 레거시 BP Skill_Aim::Released — 블루프린트에서 조준 해제 로직 오버라이드. */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Aim")
	void Released();
};
