#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/CAnimal.h"
#include "Blueprint/UserWidget.h"
#include "CDragon.generated.h"

class UAnimMontage;
class UCameraShakeBase;
class UCFlyComponent;
class UNiagaraSystem;
class UCSystemMessageComponent;
class USceneComponent;
class UInputComponent;

/** 드래곤 탈 것 / 보스 규칙 — 과거 BP_Dragon_AI 를 ACDragon 단일 클래스로 옮김. */
UCLASS()
class YJJACTIONCPPUE5_API ACDragon : public ACAnimal
{
	GENERATED_BODY()

public:
	ACDragon();

protected:
	virtual void BeginPlay() override;

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCFlyComponent> FlyComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Neck;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Components",
		meta = (DisplayName = "SystemMessageComponent"))
	TObjectPtr<UCSystemMessageComponent> SystemMessageComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "컴포넌트", meta = (DisplayName = "Rider Weapon"))
	TObjectPtr<UObject> RiderWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "컴포넌트", meta = (DisplayName = "Rider Magic"))
	TObjectPtr<UObject> RiderMagic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
	TSubclassOf<AActor> DragonWeaponClass;

	UPROPERTY(BlueprintReadOnly, Category = "Weapons", meta = (DisplayName = "Dragon Weapon"))
	TObjectPtr<AActor> DragonWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly", meta = (DisplayName = "Up Factor"))
	float UpFactor = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fly")
	TObjectPtr<UNiagaraSystem> JumpEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groggy")
	TObjectPtr<UAnimMontage> GroggyAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groggy", meta = (DisplayName = "Max Groggy"))
	double MaxGroggy = 2000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groggy")
	double Groggy = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groggy", meta = (DisplayName = "Groggy Time"))
	double GroggyTime = 8.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Groggy")
	bool InGroggy = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (DisplayName = "Info"))
	TObjectPtr<UUserWidget> BossInfoUi;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (DisplayName = "Hp Bar"))
	TObjectPtr<UUserWidget> HpBarUi;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (DisplayName = "Groggy Bar"))
	TObjectPtr<UUserWidget> GroggyBarUi;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (DisplayName = "Eye Effect"))
	TSubclassOf<AActor> EyeEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (ToolTip = "SetupCinematic(true) 종료 후 SetGroupIndex 인자(레거시 BP 디폴트 5)."))
	int32 CinematicBossGroupIndex = 5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Landing", meta = (DisplayName = "Land Camera Shake Class"))
	TSubclassOf<UCameraShakeBase> LandCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Death|Dragon", meta = (DisplayName = "Death Soul Follower Class"))
	TSubclassOf<AActor> DragonDeathSoulFollowerClass;

	UFUNCTION(Server, Reliable, WithValidation, Category = "Dragon|Weapons")
	void Server_RequestDragonWeaponSkill(uint8 InSkillEnumValue);

	/** BT·내부 경로에서 DragonWeapon 에 스킬 바이트를 넘길 때 — 서버 검증 후 `Skill()` 로 위임. */
	void InvokeDragonWeaponSkill(uint8 InSkillEnumValue);

	/** 레거시 DragonWeapon `FireFlying` 등 — Neck 을 스켈레탈 소켓에 스냅 부착한다(Fire_Ground / Fire_Air 공용). */
	UFUNCTION(BlueprintCallable, Category = "Dragon|Neck")
	void ApplyNeckAttachToDragonMeshSocket(FName SocketName);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cinematic")
	void SetupCinematic(bool OnOff);

	UFUNCTION(BlueprintCallable, Category = "Dragon|CharacterInfo", meta = (DisplayName = "Set Group Index"))
	void SetGroupIndex(int32 InIndex);

	UFUNCTION(BlueprintPure, Category = "Dragon|Fly", meta = (DisplayName = "IsFlying"))
	bool IsFlying() const;

	UFUNCTION(BlueprintCallable, Category = "Dragon|Groggy", meta = (DisplayName = "SetGroggy"))
	void BP_ClearGroggyGaugeOnly();

	UFUNCTION(BlueprintCallable, Category = "Dragon|Groggy")
	void SetGroggyDamage();

	UFUNCTION(BlueprintCallable, Category = "Dragon|Groggy", meta = (DisplayName = "End Groggy"))
	void End_Groggy();

	UFUNCTION(BlueprintCallable, Category = "Dragon|UI")
	void UpdateHp();

	UFUNCTION(BlueprintCallable, Category = "Dragon|UI")
	void UpdateGroggy();

	UFUNCTION(BlueprintCallable, Category = "Dragon|UI")
	void SpawnMessage();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|UI")
	FText DragonSpawnAnnouncementMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|UI")
	double DragonSpawnAnnouncementDurationSeconds = 5.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dragon|Groggy", meta = (ClampMin = "1.0"))
	double GroggyDamageDivisorFromHitDamage = 5.0;

	virtual void InvokeHittedEffects() override;

	virtual void Dead() override;
	virtual void End_Dead() override;

	UFUNCTION(BlueprintCallable, Category = "Dragon|Footstep", meta = (DisplayName = "Land Footstep Extras"))
	void PlayDragonFootstepLandCameraShakeAndWeapon();

private:
	void SpawnDragonWeaponFromClassIfConfigured();
	void SyncFlyVerticalStrengthFromUpFactor();

	void BindJumpEffectIntoLandEffectFallback();

	bool TryFireDragonWeaponLandAttackBlueprint(const TCHAR* CallerContextTag);

	void PlayDragonLandCameraShakePlayer0Local();

	void ApplyNeckAttachToDragonMeshSocketFireGround();

	void InvokeAnimalLandingAudioVisualOnlyForDragon();

	void RefreshDragonBossHpBarWidgets();
	void RefreshDragonGroggyBarWidgets();

	void DragonTrySetWeaponInActionBlueprint(const bool InAction);

	UFUNCTION()
	void Dragon_OnGroggyRecoverTimer_EndGroggy();

	UFUNCTION()
	void Dragon_OnEndDeadAfterSoulDelay();

	UFUNCTION()
	void Dragon_InputDragonRoarPressed();

	UFUNCTION()
	void Dragon_InputDragonFirePressed();

	FTimerHandle DragonEndDeadDelayTimer;

	FTimerHandle DragonGroggyRecoverTimerHandle;

	void Dragon_InputAxis_MoveForward(float AxisValue);
	void Dragon_InputAxis_MoveRight(float AxisValue);
	void Dragon_InputAxis_FlyUp(float AxisValue);
	void Dragon_InputAction_Jump();

	bool Dragon_ShouldApplyMovingGraphAirAssist() const;
};
