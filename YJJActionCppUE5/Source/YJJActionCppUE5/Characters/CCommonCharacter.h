#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CInterface_CharacterState.h"
#include "Interfaces/CInterface_CharacterBody.h"
#include "Interfaces/CInterface_IK.h"
#include "Commons/CGameInstance.h"
#include "Weapons/CWeaponStructures.h"
#include "Animation/AnimInstance.h"
#include "CCommonCharacter.generated.h"

class UCStateComponent;
class UCMovementComponent;
class UCMontagesComponent;
class UCCharacterInfoComponent;
class UCCharacterStatComponent;
class UCWeaponStructures;
class ACAnimal_AI;
class USoundBase;
class UFXSystemAsset;
class UWidgetComponent;
class UCUserWidget_Custom;
class USceneComponent;
class UCUserWidget_EnemyBar;
class USpringArmComponent;
class UCTargetingComponent;
class UCUserWidget_HUD;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMount, ACCommonCharacter*, Object);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUnmount);

USTRUCT()
struct FDamageData
{
	GENERATED_BODY()

	float Power;
	TWeakObjectPtr<ACCommonCharacter> Attacker;
	TWeakObjectPtr<AActor> Causer;
	FActDamageEvent Event;
};

UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACCommonCharacter :
	public ACharacter,
	public ICInterface_CharacterState,
	public ICInterface_CharacterBody,
	public ICInterface_IK
{
	GENERATED_BODY()

public:
	ACCommonCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void Landed(const FHitResult& Hit) override;

public:
	virtual float TakeDamage(
		float DamageAmount, 
		FDamageEvent const& DamageEvent,
		AController* EventInstigator, 
		AActor* DamageCauser) override;

protected:
	virtual void Rise() override;
	virtual void Land() override;
	virtual void Hit() override;
	virtual void Dead() override;

	void OnDestroyDelayTimer();

public:
	virtual void End_Hit() override;
	virtual void End_Dead() override;
	virtual void End_Rise() override;

	FORCEINLINE void SetbRiding(const bool InbRiding) { bRiding = InbRiding; }
	FORCEINLINE constexpr bool GetbRiding() const { return bRiding; }

	virtual TObjectPtr<USpringArmComponent> GetSpringArm() const
		PURE_VIRTUAL(ACCommonCharacter::GetSpringArm, return nullptr;);

	virtual TObjectPtr<UCTargetingComponent> GetTargetingComp() const
		PURE_VIRTUAL(ACCommonCharacter::GetTargetingComp, return nullptr;);

private:
	UFUNCTION()
	void RestoreColor();

	void ApplyEmbeddedWidgetClassesIfNeeded();

public:
	// 라이딩 중 PlayerController와 AIController 전환을 위해 현재 컨트롤러를 저장한다.
	void SetMyCurController(const TWeakObjectPtr<AController> InController);
	FORCEINLINE TWeakObjectPtr<AController> GetMyCurController() const { return TWeakObjectPtr<AController>(CurController.Get()); }

public:
	UFUNCTION()
	void SetTickLerp(FRotator InRotator);

	void TogglebTickLerpForTarget();

public:
	void SetMousePos(const FVector2D InPos);
	FORCEINLINE const FVector2D& GetMousePos() const { return MousePos; }

public:
	void SetInteractor(TObjectPtr<ACCommonCharacter> InCharacter);
	FORCEINLINE TObjectPtr<ACCommonCharacter> GetInteractor() const { return Interactor; };

	FORCEINLINE TObjectPtr<UFXSystemAsset> GetLandEffect() const { return LandEffect; }
	FORCEINLINE constexpr float GetLandEffectScaleFactor() const { return LandEffectScaleFactor; }

public:
	// PrevType 기준으로 이동/탑승 상태를 복구한다. (구 BP_Character::RestoreState)
	// BlueprintNativeEvent + 이름 RestoreState 는 BP 에 '이벤트 구현' 슬롯이 남고, 삭제만 해도 에셋 직렬화가 깨지는 경우가 많다.
	// UFunction 실제 이름은 분리하고, 블루프린트에서는 DisplayName 으로 "Restore State" 로 표시한다.
	UFUNCTION(BlueprintCallable, Category = "세팅", meta = (DisplayName = "Restore State"))
	void ApplyRestoreStateFromPrevMode();

public:
	void InputAction_Interact();

	// BP_Player::GetHUD 와 동일: 로컬 PC → EnsureHUD. 위젯 생성 등 부작용이 있으므로 Callable 로 둔다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	UCUserWidget_HUD* EnsureHUDWidget();

	// EnsureHUDWidget 과 동일 값. 읽기만 할 때 Pure 로 쓸 수 있으나 내부적으로 Ensure 를 호출한다.
	UFUNCTION(BlueprintPure, Category = "UI")
	UCUserWidget_HUD* GetPlayerHUDWidget() const;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	FMount OnMount;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	FUnmount OnUnmount;

	// BP_Character 변수를 네이티브로 옮김 — 애님·상호작용·피격·AI가 동일 프로퍼티를 참조하도록 통일한다.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "세팅")
	bool bIsRotating = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact", meta = (MultiLine = "true"))
	TObjectPtr<AActor> InteractableActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact", meta = (MultiLine = "true"))
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact", meta = (MultiLine = "true", ToolTip = "RidingAnimal, Bow 등에서 쓰는 현재 상호작용 액터."))
	TObjectPtr<AActor> CurInteractingActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact", meta = (MultiLine = "true"))
	TObjectPtr<AController> CurController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit", meta = (MultiLine = "true"))
	TObjectPtr<ACCommonCharacter> Attacker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit", meta = (MultiLine = "true"))
	TObjectPtr<AActor> Causer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit", meta = (MultiLine = "true"))
	FHitData HitData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit", meta = (MultiLine = "true"))
	FVector HitPoint = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "Hit", meta = (MultiLine = "true", AllowPrivateAccess = "true"))
	float ConstLaunchingBack = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (MultiLine = "true"))
	TObjectPtr<AController> ControllerSave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (MultiLine = "true"))
	FVector NextMovingPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (MultiLine = "true"))
	FCCharacterInfo CharacterInfo;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCStateComponent> StateComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCMovementComponent> MovementComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCMontagesComponent> MontagesComp;

	UPROPERTY(VisibleAnywhere, Category = "Status")
	TObjectPtr<UCCharacterInfoComponent> CharacterInfoComp;
	
	UPROPERTY(VisibleAnywhere, Category = "Status")
	TObjectPtr<UCCharacterStatComponent> CharacterStatComp;


	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TObjectPtr<USceneComponent> TargetingPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TObjectPtr<UWidgetComponent> TargetingWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Targeting")
	TSubclassOf<UCUserWidget_Custom> TargetingWidget;


	UPROPERTY(EditDefaultsOnly, Category = "Info")
	TObjectPtr<USceneComponent> InfoPoint;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	TObjectPtr<UWidgetComponent> InfoWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	TSubclassOf<UCUserWidget_EnemyBar> InfoWidget;


	UPROPERTY(EditDefaultsOnly, Category = "Land")
	TObjectPtr<USoundBase> LandSound;

	UPROPERTY(EditDefaultsOnly, Category = "Land")
	TObjectPtr<UFXSystemAsset> LandEffect;

	UPROPERTY(EditAnywhere, Category = "Land")
	float LandEffectScaleFactor = 1.0f;


	UPROPERTY(EditAnywhere, Category = "Color")
	FLinearColor OriginColor = FLinearColor::White;

private:
	UPROPERTY(VisibleAnywhere, Category = "Key")
	FVector2D MousePos;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<ACCommonCharacter> Interactor;

	UPROPERTY(VisibleAnywhere, Category = "Ride")
	bool bRiding;

protected:
	FTimerHandle RestoreColor_TimerHandle;
	FTimerHandle DestroyDelay_TimerHandle;
	FDamageData Damage;

private:
	bool bTickLerpForTarget = false;
	FRotator TargetRotator = FRotator(0, 0, 0);
};
