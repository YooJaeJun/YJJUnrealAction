#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/CInterface_CharacterState.h"
#include "Interfaces/CInterface_CharacterBody.h"
#include "Interfaces/CInterface_IK.h"
#include "Interfaces/CInterface_PlayerPossess.h"
#include "Interfaces/CInterface_CharacterMenu.h"
#include "Interfaces/CInterface_CharacterGameplay.h"
#include "Camera/CameraShakeBase.h"
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
class UTextRenderComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMount, ACCommonCharacter*, Object);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUnmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteract);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnIsDeadCharacter);

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
	public ICInterface_IK,
	public ICInterface_PlayerPossess,
	public ICInterface_CharacterMenu,
	public ICInterface_CharacterGameplay
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

	// AnimNotify·커스텀 이벤트가 BP_End_Hitted 대신 호출. 피격 중일 때만 이전 이동/탑승 모드를 복구한다.
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (DisplayName = "End Hitted (AI Restore)"))
	void EndHitted_ApplyRestoreIfStillInHit();

	// BP 커스텀 이벤트 End_Hitted 와 동일 — 애님 노티파이에서 호출.
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (DisplayName = "End Hitted"))
	void End_Hitted();

	// BP I_PlayerPossess::TogglePossess — 로컬 PlayerIndex 0. 전용 서버에서는 조종할 PC 가 없으면 무시한다.
	UFUNCTION(BlueprintCallable, Category = "탑승")
	void TogglePlayerPossessLocal(const bool bEnable);

	// BP_Character::GetCurrentSpeedType — CharacterMovement.MaxWalkSpeed 와 Movement 의 Speeds[] 를 비교한다.
	// 기존 User Defined Enum ESpeedType 대신 네이티브 CESpeedType 을 쓰도록 블루프린트 핀을 교체한다.
	UFUNCTION(BlueprintPure, Category = "세팅")
	CESpeedType GetCurrentSpeedType(float Tolerance = 1.0f) const;

	// BP_Player / BP_Animal::CanMove — exec 그래프 호환(Callable). |축|>0.5 일 때만 MovingComponent::IsCanMove.
	UFUNCTION(BlueprintCallable, Category = "Utilities")
	void CanMove(double InAxis, bool& OutCanMove) const;

	// BP_Character::LaunchBack — 뒤로 밀기 + Z 보정은 BP 와 동일하게 기본 1000.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void LaunchBack();

	// BP_Character::LaunchHit — HitData.Launch 만큼 전방 반대 방향으로 LaunchCharacter.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void LaunchHit();

	// BP_Character::SetRotation — 타깃 액터만 바라보는 요(Yaw)만 적용.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetRotation(AActor* TargetActor, bool bTeleportPhysics = false);

	// BP_Character::PlayParticle / PlaySound — 기본은 멤버 HitData 사용(브레이크 노드 없이 C++ 호출만으로 동작).
	UFUNCTION(BlueprintCallable, Category = "Hit", meta = (DisplayName = "Play Particle"))
	void PlayParticle();

	UFUNCTION(BlueprintCallable, Category = "Hit", meta = (DisplayName = "Play Sound"))
	void PlayHitSound();

	// 레거시 BP_Character 노드명 "Play Sound" — 멤버 이름만 BP 가 기대할 때 대비.
	UFUNCTION(BlueprintCallable, Category = "Hit")
	void PlaySound();

	// BP_Character::SetDamagedInfo — I_Damage.Damaged 에서 피격 맥락을 멤버에 저장한다.
	UFUNCTION(BlueprintCallable, Category = "Hit", meta = (DisplayName = "Set Damaged Info"))
	void SetDamagedInfo(ACCommonCharacter* InAttacker, AActor* InCauser, FHitData InHitData, FVector InHitPoint);

	// 로컬 조종/탑승 시 피드백용 카메라 쉐이크. 전용 서버에서는 생략한다.
	UFUNCTION(BlueprintCallable, Category = "Hit", meta = (DisplayName = "Play Camera Shake"))
	virtual void PlayCameraShake();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit")
	TSubclassOf<UCameraShakeBase> HitReactionCameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit")
	float HitReactionCameraShakeScale = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void RenderStateText();

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DontRenderStateText();

	// BP 스펠링 유지(Bilboard). 카메라(로컬 0)를 향하도록 디버그 텍스트 컴포넌트 회전.
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void BilboardStateText();

public:
	virtual void InputAction_Interact();

	// BP_Player::GetHUD 와 동일: 로컬 PC → EnsureHUD. 위젯 생성 등 부작용이 있으므로 Callable 로 둔다.
	UFUNCTION(BlueprintCallable, Category = "UI")
	UCUserWidget_HUD* EnsureHUDWidget();

	// EnsureHUDWidget 과 동일 값. 읽기만 할 때 Pure 로 쓸 수 있으나 내부적으로 Ensure 를 호출한다.
	UFUNCTION(BlueprintPure, Category = "UI")
	UCUserWidget_HUD* GetPlayerHUDWidget() const;

	// 구 I_Character / BP_Character — 입력·래핑 그래프가 self 에서 직접 호출. BP 에서 오버라이드해 UI 토글을 유지한다.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void OnEquipMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void OffEquipMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void OnMagicMenu();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|Menu")
	void OffMagicMenu();

	// 블루프린트가 캐릭터 self 에서 IsRiding 을 호출하는 레거시 노드 호환.
	UFUNCTION(BlueprintPure, Category = "State")
	bool IsRiding() const { return GetbRiding(); }

	// 레거시 BP_Player::SetIdle — 타깃이 캐릭터로 남은 노드가 StateComp 로 위임한다.
	UFUNCTION(BlueprintCallable, Category = "State", meta = (DisplayName = "Set Idle"))
	void SetIdle();

public:
	// 모든 ActorComponent DefaultSubobject FName 은 /Game 의 BP 컴포넌트(같은 슬롯 이름)와 겹치면 Fatal 이 난다. YJJ 접두사로 통일.
	// 블루프린트 Variable Get "StateComponent" 가 자식 BP_Player 에서도 해석되도록 public 에 둔다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (DisplayName = "State Component"))
	TObjectPtr<UCStateComponent> StateComp;

	UPROPERTY(BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCStateComponent> StateComponent;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	FMount OnMount;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	FUnmount OnUnmount;

	// BP Event Dispatchers: OnInteract(Interact 눌림), IsDead(사망 처리 진입 시).
	UPROPERTY(BlueprintAssignable, Category = "Character|Events")
	FOnInteract OnInteract;

	UPROPERTY(BlueprintAssignable, Category = "Character|Events", meta = (DisplayName = "IsDead"))
	FOnIsDeadCharacter IsDead;

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

	// BP LaunchBack 의 LaunchVelocity Z 스플릿 핀 기본값.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit")
	float LaunchBackVerticalImpulse = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (MultiLine = "true"))
	TObjectPtr<AController> ControllerSave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (MultiLine = "true"))
	FVector NextMovingPoint = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (MultiLine = "true"))
	FCCharacterInfo CharacterInfo;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCMovementComponent> MovementComp;

	// 예전 BP 변수명 "MovingComponent" — 항상 MovementComp 와 동일 인스턴스다.
	UPROPERTY(BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCMovementComponent> MovingComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCMontagesComponent> MontagesComp;

	UPROPERTY(VisibleAnywhere, Category = "Status")
	TObjectPtr<UCCharacterInfoComponent> CharacterInfoComp;
	
	UPROPERTY(VisibleAnywhere, Category = "Status")
	TObjectPtr<UCCharacterStatComponent> CharacterStatComp;

	// BP_Character 가 Variables 에 StateTextRender 를 가지고 있으면 부모에 동일 UPROPERTY 이름을 두면 SKEL 양쪽에서
	// 프로퍼티가 중복되어 컴파일 ICE 가 난다. 네이티브 전용 이름으로 분리한다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug", meta = (DisplayName = "State Text (Native)"))
	TObjectPtr<UTextRenderComponent> StateTextComponent;

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

	// ICInterface_PlayerPossess
	virtual void TogglePossess_Implementation(bool InEnable) override;

	// ICInterface_CharacterMenu (I_Character 메뉴 이벤트 포팅)
	virtual void HoveredEquipMenu_Implementation(const FString& InName) override;
	virtual void UnHoveredEquipMenu_Implementation(const FString& InName) override;
	virtual void ClickedEquipMenu_Implementation(const FString& InName) override;
	virtual void HoveredMagicMenu_Implementation(const FString& InName) override;
	virtual void UnHoveredMagicMenu_Implementation(const FString& InName) override;
	virtual void ClickedMagicMenu_Implementation(const FString& InName) override;

	// ICInterface_CharacterGameplay (구 I_Character 세팅 이벤트)
	virtual void Footstep_Implementation() override;
	virtual void Rewarded_Implementation() override;
	virtual void Damaged_Implementation(float DamageAmount) override;
	virtual void StartInteraction_Implementation(AActor* InteractionTarget) override;
	virtual void EndInteraction_Implementation() override;
	virtual int32 GetAction_Implementation() override;
};
