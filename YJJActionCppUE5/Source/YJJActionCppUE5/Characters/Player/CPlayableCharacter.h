#pragma once
#include "CoreMinimal.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "CPlayableCharacter.generated.h"

class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UCAnimInstance_Character;
class UCMontagesComponent;
class UCCamComponent;
class UCTargetingComponent;
class UCWeaponComponent;
class UCGameUIComponent;
class UCInventoryComponent;
class UCPlacementComponent;
class UWidgetComponent;
class ACGameMode;
class UCUserWidget_EquipMenu;
class UCUserWidget_MagicMenu;
class UCUserWidget_Interaction;

UCLASS()
class YJJACTIONCPPUE5_API ACPlayableCharacter :
	public ACCommonCharacter
{
	GENERATED_BODY()

public:
	ACPlayableCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// BP_Player::SetStatusUI — 스탯 UI 를 CharacterStatComp 기준으로 동기화하고 HUD 를 표시한다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetStatusUI();

	// BP_Player::SetMenuUI — HUD 하위 Equip/Magic/Interaction 을 정리하고 장착 델리게이트를 캐릭터로 연결한다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetMenuUI();

	UFUNCTION()
	void EquipWeaponFromUI(const CEWeaponType InNewType);

	UFUNCTION()
	void EquipMagicFromUI(const CEWeaponType InNewType);

private:
	void InputAction_Avoid();

	virtual void Avoid() override;
	virtual void Hit() override;

	virtual void End_Avoid() override;
	virtual void End_Rise() override;
	virtual void End_Hit() override;

private:
	UFUNCTION()
	void OnStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType);

	UFUNCTION()
	void OnHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType);

public:
	virtual TObjectPtr<USpringArmComponent> GetSpringArm() const override;
	virtual TObjectPtr<UCTargetingComponent> GetTargetingComp() const override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCWeaponComponent> WeaponComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCTargetingComponent> TargetingComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCCamComponent> CamComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCGameUIComponent> GameUIComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCInventoryComponent> InventoryComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCPlacementComponent> PlacementComp;

	UPROPERTY(EditAnywhere, Category = "Mode")
	TWeakObjectPtr<ACGameMode> GameMode;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector2D PitchRange = FVector2D(-40, +40);
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	TArray<float> Speeds{ 200, 500, 800 };

private:
	CEHitType CurHitType = CEHitType::Common;

	UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCUserWidget_EquipMenu> MenuEquipWidget;

	UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCUserWidget_MagicMenu> MenuMagicWidget;

	UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCUserWidget_Interaction> MenuInteractionWidget;
};