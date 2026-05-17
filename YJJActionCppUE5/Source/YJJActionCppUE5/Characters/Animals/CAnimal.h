#pragma once

#include "CoreMinimal.h"
#include "Characters/CCommonCharacter.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"
#include "Blueprint/UserWidget.h"
#include "Chaos/ChaosEngineInterface.h"
#include "NiagaraSystem.h"
#include "Particles/ParticleSystem.h"
#include "Camera/CameraShakeBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/CStateComponent.h"
#include "CAnimal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnimalRidingSimpleEvent);

class UWidgetComponent;
class USceneComponent;
class UInputComponent;
class USpringArmComponent;
class UCameraComponent;
class UCCamComponent;
class UCGameUIComponent;
class UCRidingComponent;
class UCWeaponComponent;
class UCPatrolComponent;
class UBoxComponent;
class UBehaviorTree;
class UCTargetingComponent;

// BP_Animal 등 — 탈 것/카메라/BT 등 공통 규칙은 ACAnimal 에 둠(과거 별도 AI 서브타입 로직 포함).
UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACAnimal : public ACCommonCharacter
{
	GENERATED_BODY()

public:
	ACAnimal();

protected:
	virtual void BeginPlay() override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

	virtual void InputAction_Interact() override;

	// HpBarWidget(WidgetComponent) — 없으면 SetHpUI 는 HpBar_NPC 갱신만 한다.
	virtual UWidgetComponent* GetAnimalHpBarWidgetComponent() const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Animal", meta = (MultiLine = "true"))
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Animal", meta = (MultiLine = "true"))
	double Exp = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Animal", meta = (MultiLine = "true"))
	double MaxExp = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Animal", meta = (MultiLine = "true"))
	double RewardExp = 30.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Animal", meta = (MultiLine = "true"))
	double MaxHp = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Animal", meta = (MultiLine = "true"))
	double Hp = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Animal", meta = (MultiLine = "true"))
	TObjectPtr<UUserWidget> HpBar_NPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status|Animal", meta = (MultiLine = "true", DisplayName = "On UIInfo"))
	bool bOnUIInfo = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit|Animal", meta = (MultiLine = "true"))
	TArray<TObjectPtr<AActor>> DilationActors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit|Animal", meta = (MultiLine = "true"))
	double CurHeight = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit|Animal", meta = (MultiLine = "true", DisplayName = "Flying Hitted"))
	bool bFlyingHitted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims|Animal", meta = (MultiLine = "true"))
	TArray<TObjectPtr<UAnimMontage>> AttackAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims|Animal", meta = (MultiLine = "true"))
	TObjectPtr<UAnimMontage> LandAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims|Animal", meta = (MultiLine = "true"))
	TObjectPtr<UAnimMontage> HitAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anims|Animal", meta = (MultiLine = "true"))
	TObjectPtr<UAnimMontage> DeadAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Animal", meta = (MultiLine = "true"))
	TObjectPtr<USoundBase> MountSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Animal", meta = (MultiLine = "true"))
	TObjectPtr<USoundBase> UnmountSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Animal", meta = (MultiLine = "true"))
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Animal", meta = (MultiLine = "true"))
	TObjectPtr<USoundBase> DeadSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds|Animal", meta = (MultiLine = "true"))
	TObjectPtr<USoundBase> HitUniqueSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit|Animal", meta = (MultiLine = "true"))
	TObjectPtr<UNiagaraSystem> BloodEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Animal", meta = (MultiLine = "true"))
	TSubclassOf<AActor> BloodActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blood|Animal", meta = (MultiLine = "true"))
	TObjectPtr<USoundBase> BloodSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials|Animal", meta = (MultiLine = "true"))
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Death|Animal", meta = (MultiLine = "true"))
	TObjectPtr<UParticleSystem> DeathSoulEmitterTemplate;

	UPROPERTY(BlueprintAssignable, Category = "Reward|Animal", meta = (DisplayName = "OnUpdateLevel_Riding"))
	FOnAnimalRidingSimpleEvent OnUpdateLevel_Riding;

	UPROPERTY(BlueprintAssignable, Category = "Reward|Animal", meta = (DisplayName = "OnUpdateExp_Riding"))
	FOnAnimalRidingSimpleEvent OnUpdateExp_Riding;

	UPROPERTY(BlueprintAssignable, Category = "Reward|Animal", meta = (DisplayName = "OnUpdateHp_Riding"))
	FOnAnimalRidingSimpleEvent OnUpdateHp_Riding;

	UPROPERTY(EditDefaultsOnly, Category = "UI|Animal")
	TSubclassOf<UUserWidget> AnimalHpBarWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "피직스|Animal", meta = (MultiLine = "true"))
	TArray<TObjectPtr<USoundBase>> FootstepSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "피직스|Animal", meta = (MultiLine = "true"))
	TArray<TObjectPtr<UNiagaraSystem>> FootstepEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "피직스|Animal", meta = (MultiLine = "true"))
	TArray<TObjectPtr<UNiagaraSystem>> LandEffects;

	// --- 탑승 카메라·타게팅·UI 등 야수 전용 컴포넌트·데이터 ---
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCCamComponent> CamComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCTargetingComponent> TargetingComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCGameUIComponent> GameUIComp;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Components",
		meta = (DisplayName = "RidingComponent"))
	TObjectPtr<UCRidingComponent> AnimalRidingComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UCWeaponComponent> WeaponComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI", meta = (MultiLine = "true"))
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat|AnimalWeapon", meta = (MultiLine = "true", DisplayName = "Weapon Class"))
	TSubclassOf<AActor> AnimalWeaponClass;

	/** false 이면 BeginPlay 가 AnimalWeapon 디스크 기본 경로로 클래스를 채우지 않는다(CDragon 등 무기 블프 별도). */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|AnimalWeapon")
	bool bAnimalFillDefaultWeaponClassFromDiskWhenUnset = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|AnimalWeapon", meta = (MultiLine = "true", DisplayName = "Weapon"))
	TObjectPtr<AActor> AnimalWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "SpringArm"))
	TObjectPtr<USpringArmComponent> AnimalSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Camera"))
	TObjectPtr<UCameraComponent> AnimalViewCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	TArray<float> Speeds{ 400, 1000, 1600 };

	UPROPERTY(VisibleAnywhere, Category = "AI", meta = (DisplayName = "Patrol Component"))
	TObjectPtr<UCPatrolComponent> PatrolComp;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
	TObjectPtr<USceneComponent> MountLeftPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
	TObjectPtr<USceneComponent> MountRightPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
	TObjectPtr<USceneComponent> MountBackPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding", meta = (DisplayName = "RiderPoint"))
	TObjectPtr<USceneComponent> AnimalRiderPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
	TObjectPtr<USceneComponent> UnmountPoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding", meta = (DisplayName = "EyePoint"))
	TObjectPtr<USceneComponent> AnimalEyePoint;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
	TSubclassOf<AActor> EyeClass;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
	TObjectPtr<AActor> Eye;

	UPROPERTY(VisibleAnywhere, Category = "Riding")
	TObjectPtr<UBoxComponent> InteractionCollision;

	UPROPERTY(VisibleAnywhere, Category = "UI", meta = (DisplayName = "Scene"))
	TObjectPtr<USceneComponent> HpBarSceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "HpBarWidget"))
	TObjectPtr<UWidgetComponent> HpBarWidgetComp;

	UFUNCTION(BlueprintCallable, Category = "Status|Animal", meta = (DisplayName = "Set Hp"))
	void SetHp();

	UFUNCTION(BlueprintCallable, Category = "UI|Animal", meta = (DisplayName = "Set Hp UI"))
	void SetHpUI();

	UFUNCTION(BlueprintCallable, Category = "UI|Animal", meta = (DisplayName = "Update Hp NPC"))
	void UpdateHp_NPC();

	/** 부모 `ACCommonCharacter::InvokeHittedEffects` 에 UFUNCTION 이 있음 — 재선언 시 UFUNCTION 블록을 붙이면 UHT 에러난다. */
	virtual void InvokeHittedEffects() override;

	UFUNCTION(BlueprintCallable, Category = "세팅|Animal")
	void FootstepAt(bool bLeftOrRight, EPhysicalSurface SurfaceType, FVector StepLocation);

	UFUNCTION(BlueprintCallable, Category = "세팅|Animal")
	void LandAt(EPhysicalSurface SurfaceType, FVector StepLocation);

	UFUNCTION(BlueprintCallable, Category = "Combat|Animal")
	void SetDamage(float InDamage, bool& OutHittedOrDead);

	UFUNCTION(BlueprintCallable, Category = "Reward|Animal", meta = (DisplayName = "Level Up"))
	void LevelUp();

	UFUNCTION(BlueprintCallable, Category = "Combat|Animal", meta = (DisplayName = "Load Prev State"))
	void LoadPrevState();

	UFUNCTION(BlueprintCallable, Category = "Combat|Animal", meta = (DisplayName = "Play Hit Anim"))
	void PlayHitAnim();

	UFUNCTION(BlueprintCallable, Category = "Hit|Animal", meta = (DisplayName = "Play Hit Unique Sound"))
	void PlayHitUniqueSound();

	UFUNCTION(BlueprintCallable, Category = "Hit|Animal", meta = (DisplayName = "Spawn Blood"))
	void SpawnBlood();

	UFUNCTION(BlueprintCallable, Category = "Hit|Animal", meta = (DisplayName = "Play Hit Stop"))
	void PlayHitStop();

	UFUNCTION(BlueprintCallable, Category = "Hit|Animal", meta = (DisplayName = "Restore Time Dilation"))
	void RestoreTimeDilation();

	virtual void PlayCameraShake() override;

	UFUNCTION(BlueprintCallable, Category = "Reward|Animal")
	void ApplyRewardedEvent(ACharacter* Invoker, double InExp, int32 BuffIndex);

	UFUNCTION(BlueprintCallable, Category = "Reward|Animal")
	void LevelUpAnimal();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animal|IK", meta = (DisplayName = "Toggle IK"))
	void ToggleIK();

	UFUNCTION(BlueprintCallable, Category = "Utilities", meta = (DisplayName = "Get Control Direction"))
	void GetControlDirection(FVector& OutForward, FVector& OutRight) const;

	UFUNCTION(
		BlueprintNativeEvent,
		BlueprintCallable,
		Category = "Animal|Locomotion",
		meta = (DisplayName = "Get Desired Movement"))
	void GetAnimalDesiredMovement(UPARAM(DisplayName = "Out Movement") FVector& OutMovement);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animal|IK")
	void SetFootLocation(bool bLeftFoot, FVector WorldLocation);

	void SetZoomMinRange(const float InMinRange) const;
	void SetZoomMaxRange(const float InMaxRange) const;

	FORCEINLINE TObjectPtr<UBoxComponent> GetInteractionCollision() const { return InteractionCollision; }
	FORCEINLINE TObjectPtr<USceneComponent> GetMountLeftPoint() const { return MountLeftPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetMountRightPoint() const { return MountRightPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetMountBackPoint() const { return MountBackPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetRiderPoint() const { return AnimalRiderPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetUnmountPoint() const { return UnmountPoint; }
	FORCEINLINE TObjectPtr<USceneComponent> GetEyePoint() const { return AnimalEyePoint; }

	FORCEINLINE TSubclassOf<AActor> GetEyeActorClass() const { return EyeClass; }
	FORCEINLINE TObjectPtr<AActor> GetSpawnedEyeActor() const { return Eye; }

	virtual TObjectPtr<USpringArmComponent> GetSpringArm() const override;
	virtual TObjectPtr<UCTargetingComponent> GetTargetingComp() const override;
	FORCEINLINE TObjectPtr<UCameraComponent> GetCamera() const { return AnimalViewCamera; }
	FORCEINLINE TObjectPtr<UCCamComponent> GetZoomComp() const { return CamComp; }

	UBehaviorTree* GetAnimalBehaviorTreeForController() const;

protected:
	virtual void Hit() override;
	virtual void Dead() override;
	virtual void End_Dead() override;

private:
	UFUNCTION()
	void OnMountedAnimalStateTypeChanged(const CEStateType InPrevType, const CEStateType InNewType);

	UFUNCTION()
	void OnMountedAnimalHitStateTypeChanged(const CEHitType InPrevType, const CEHitType InNewType);

	void SpawnAnimalWeaponFromClassIfConfigured();

	/** ctor 의 ConstructorHelpers/동기 블프 로드는 BP_Animal CDO·AnimalWeapon 과 순환해 AsyncLoading2 assertion 나기 쉬움 → BeginPlay 에서 보충. */
	void AnimalEnsureLandingAndDeferredBlueprintAssetsLoadedAfterCommonBeginPlay();

	void TryGrantKillRewardToAttacker();

	UFUNCTION()
	void OnAnimalDeathAfterRewardDelay();

	UFUNCTION()
	void OnAnimalDeathSpawnSoulAndDestroy();

	bool bAnimalDeathSequenceStarted = false;

	FTimerHandle AnimalDeathTimer_RewardDelay;
	FTimerHandle AnimalDeathTimer_SoulFx;

protected:
	virtual void Footstep_Implementation(bool bLeftFoot, EPhysicalSurface SurfaceType, FVector HitLocation) override;

	virtual void RefreshAnimalHpBarWidgets();

	void ApplyEnemyHpBarPercent();

	float PlayLandMontageIfAny();

private:
	FTimerHandle HitStopRestoreTimer;
};
