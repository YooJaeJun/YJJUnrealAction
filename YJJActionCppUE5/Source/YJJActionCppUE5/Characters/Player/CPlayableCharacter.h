#pragma once
#include "CoreMinimal.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CStateComponent.h"
#include "Components/CCamComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraShakeBase.h"
#include "CPlayableCharacter.generated.h"

class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UCAnimInstance_Character;
class UCMontagesComponent;
class UCTargetingComponent;
class UCWeaponComponent;
class UCMagicComponent;
class UCGameUIComponent;
class UCInventoryComponent;
class UCPlacementComponent;
class UCSystemMessageComponent;
class UCParkourComponent;
class UWidgetComponent;
class ACGameMode;
class UCUserWidget_EquipMenu;
class UCUserWidget_MagicMenu;
class UCUserWidget_Interaction;
class UPointLightComponent;
class UNiagaraComponent;
class UChildActorComponent;
class UArrowComponent;
class USceneComponent;

UCLASS()
class YJJACTIONCPPUE5_API ACPlayableCharacter :
	public ACCommonCharacter
{
	GENERATED_BODY()

public:
	ACPlayableCharacter();

protected:
	virtual void PostInitializeComponents() override;
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

	// 구 I_Cinematic::SetupCinematic — 시네 HUD/조명 토글 그래프가 타깃으로 호출.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cinematic")
	void SetupCinematic(bool OnOff);

	// BP_Player 시네 마커용 조명 — Mesh 하위 Scene → PointLight 들 (캡슐이 아님).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cinematic", meta = (DisplayName = "Scene"))
	TObjectPtr<USceneComponent> CinematicLightScene;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Cinematic",
		meta = (DisplayName = "PointLight"))
	TObjectPtr<UPointLightComponent> NativeBpCinematicPointLightA;

	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Cinematic",
		meta = (DisplayName = "PointLight1"))
	TObjectPtr<UPointLightComponent> NativeBpCinematicPointLightB;

	// BP_Player — 캡슐 하위 방향 마커(파쿠르·에어 기준점 등). 변수명 ArrowGroup 과 BP_Player SCS 가 겹치면 SKEL 속성 ICE — NativeBp 접두 분리.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Markers", meta = (DisplayName = "ArrowGroup"))
	TObjectPtr<USceneComponent> NativeBpArrowGroup;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Markers")
	TObjectPtr<UArrowComponent> ArrowCeil;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Markers")
	TObjectPtr<UArrowComponent> ArrowCenter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Markers")
	TObjectPtr<UArrowComponent> ArrowFloor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Markers")
	TObjectPtr<UArrowComponent> ArrowLand0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Markers")
	TObjectPtr<UArrowComponent> ArrowLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Markers")
	TObjectPtr<UArrowComponent> ArrowRight;

	// 레거시 BP_Player — 활 장착(양궁) 모드 여부. (과거 IsNotBowMode 의 부정을 제거한 긍정형)
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsBowMode() const;

	// BP_Player::SetDamage — Hp 차감·UI 갱신. InDamage <= 0 이면 HitData.Damage 사용.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetDamage(float InDamage, bool& OutHittedOrDead);

	// 레거시 BP CanHitAnim(Exec) — PrevType·피격 정리 후 OutCanHitAnim 설정.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CanHitAnim(bool& OutCanHitAnim);

	// 레거시 BP 스펠링 유지(Gruad). SubWeapon 이 가드 블루프린트일 때 Guarding||Parrying.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CheckGruadOrParrying(bool& OutResult);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayHitAnim();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CancelHitAnim();

	// BP_Player::SpawnMessage — SystemMessageComponent.Play 기본 메시지.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SpawnMessage();

	// BP_Player::SetMinimap — Minimap 클래스를 현재 트랜스폼에 스폰.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetMinimap();

	// BP_Player::GetHUD — EnsureHUD 와 동일(레거시 이름 호환).
	UFUNCTION(BlueprintCallable, Category = "UI")
	UCUserWidget_HUD* GetHUD();

	// BP_Player::SetColor — 피격 빨간색 플래시 후 짧은 딜레이로 SetOriginColor.
	UFUNCTION(BlueprintCallable, Category = "Hit")
	void SetColor();

	// BP_Player::SetOriginColor — Materials 에 CharacterInfo.BodyColor 복구.
	UFUNCTION(BlueprintCallable, Category = "Hit")
	void SetOriginColor();

	// 레거시 BP — 이전 이동/상태 복구. 현재는 공통 복구 함수에 위임.
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void LoadPrevState();

	// -------------------------------------------------------------------------
	// BP_Player 변수 미러 — 에셋 Guid/이름 호환용 (값은 CharacterStatComp·HUD와 BeginPlay 에서 동기화 가능).
	// FluidForceDynamic 등 Water 플러그인 전용 UDS 는 C++ 타입이 없어 BP 서브클래스에서만 유지하거나 추후 InstancedStruct 로 이전.
	// -------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic Info", meta = (MultiLine = "true"))
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double Exp = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double MaxExp = 100.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double Hp = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double MaxHp = 1000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double Stamina = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double MaxStamina = 400.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double AccelStaminaRestore = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double DefaultStaminaRestore = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double Mana = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double MaxMana = 400.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double AccelManaRestore = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	double DefaultManaRestore = 0.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	bool EnoughStamina = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status", meta = (MultiLine = "true"))
	bool EnoughMana = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine = "true"))
	TObjectPtr<UUserWidget> HpBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine = "true"))
	TObjectPtr<UUserWidget> StaminaBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine = "true"))
	TObjectPtr<UUserWidget> ManaBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine = "true"))
	TObjectPtr<UUserWidget> LevelBar;

	// 레거시 WB_* 참조 — Menu* 위젯과 별도로 BP 그래프가 직접 캐시할 때 사용.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine = "true"))
	TObjectPtr<UUserWidget> EquipMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine = "true"))
	TObjectPtr<UUserWidget> Interaction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (MultiLine = "true"))
	TObjectPtr<UUserWidget> MagicMenu;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap", meta = (MultiLine = "true"))
	TSubclassOf<AActor> Minimap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (MultiLine = "true"))
	TObjectPtr<AActor> FluidSimFolowing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water", meta = (MultiLine = "true"))
	TSubclassOf<AActor> FluidSimClass;

	// BP_Player::MotionTrailEffect — 네이티브 속성 이름이 BP_Player SCS 과 겹치면 SKEL ICE.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trail",
		meta = (MultiLine = "true", DisplayName = "MotionTrailEffect"))
	TObjectPtr<UNiagaraComponent> NativeBpMotionTrailNiagara;

	// BP_Player — Mesh 하위 SkillCam 앵커 및 ChildActor 등. 이름이 BP SCS 과 겹치면 ICE / 인터페이스 제거 시 재구성 Fatal 이 날 수 있다.
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Camera|SkillCam",
		meta = (DisplayName = "SequenceCamera"))
	TObjectPtr<USceneComponent> NativeBpSequenceCamAnchor;

	// BP_Player 스킬 시네 카메라 — ChildActor 의 카메라 액터로 블렌드.
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Camera|SkillCam",
		meta = (MultiLine = "true", DisplayName = "SequenceCamChild"))
	TObjectPtr<UChildActorComponent> NativeBpSequenceCamChild;

	// 메인 카메라 컴포넌트 하위 — BP 의 MainCamChild 와 동일 계층.
	UPROPERTY(
		VisibleAnywhere,
		BlueprintReadOnly,
		Category = "Camera|SkillCam",
		meta = (MultiLine = "true", DisplayName = "MainCamChild"))
	TObjectPtr<UChildActorComponent> NativeBpMainCamChild;

	// BP_Player SkillSequence — UActorSequenceComponent 등, SequencePlayer 프로퍼티가 있으면 Begin_SkillCam 에서 Play 호출.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|SkillCam", meta = (MultiLine = "true",
													   DisplayName = "SkillSequence"))
	TObjectPtr<UActorComponent> NativeBpSkillSequence;

	TArray<TObjectPtr<UNiagaraSystem>> LandEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "피직스", meta = (MultiLine = "true"))
	TArray<TObjectPtr<UNiagaraSystem>> FootstepEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "피직스", meta = (MultiLine = "true"))
	TArray<TObjectPtr<USoundBase>> FootstepSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK", meta = (MultiLine = "true"))
	double LegIKAlpha = 0.0;

	// Reward 그래프에서 호출 — 이후 레벨/스탯 갱신 로직 포팅 예정.
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void UpdateLevel();

	UFUNCTION(BlueprintCallable, Category = "Reward")
	void UpdateHp();

	UFUNCTION(BlueprintCallable, Category = "Reward")
	void UpdateStamina();

	UFUNCTION(BlueprintCallable, Category = "Reward")
	void UpdateMana();

	UFUNCTION(BlueprintCallable, Category = "Reward")
	void LevelUp();

	UFUNCTION(BlueprintCallable, Category = "Reward")
	void UpdateExp();

	// BP_Player 세팅 카테고리 — 그래프를 C++ 로 이전해 컴파일 오류(끊긴 노드)를 줄인다.
	UFUNCTION(BlueprintCallable, Category = "세팅", meta = (DisplayName = "Set Material"))
	void SetMaterial();

	UFUNCTION(BlueprintCallable, Category = "세팅", meta = (DisplayName = "Set Status"))
	void SetStatus();

	UFUNCTION(BlueprintCallable, Category = "Camera", meta = (DisplayName = "Set View Pitch"))
	void SetViewPitch();

	UFUNCTION(BlueprintCallable, Category = "Status", meta = (DisplayName = "Restore Stamina"))
	void RestoreStamina();

	UFUNCTION(BlueprintCallable, Category = "Status", meta = (DisplayName = "Restore Mana"))
	void RestoreMana();

	UFUNCTION(BlueprintCallable, Category = "Status", meta = (DisplayName = "Is Enough Stamina"))
	bool IsEnoughStamina(double InConsume);

	// 레거시 Weapon_PlayAction ConsumeStamina — 스태미나가 충분할 때만 차감하고 EnoughStamina/내부 스탯·UI 동기화.
	UFUNCTION(BlueprintCallable, Category = "Status", meta = (DisplayName = "Consume Stamina"))
	bool ConsumeStamina(double InConsume);

	/** 레거시 Magic ConsumeMana 그래프 — IsEnough 검사 후 Clamp 차감·스탯·UI 동기화(Consume Stamina 와 동일 형태). */
	UFUNCTION(BlueprintCallable, Category = "Status", meta = (DisplayName = "Consume Mana"))
	bool ConsumeMana(double InConsume);

	UFUNCTION(BlueprintCallable, Category = "Status", meta = (DisplayName = "Is Enough Mana"))
	bool IsEnoughMana(double InConsume);

	UFUNCTION(BlueprintCallable, Category = "Status", meta = (DisplayName = "Not Enough Stamina"))
	void NotEnoughStamina();

	UFUNCTION(BlueprintCallable, Category = "Status", meta = (DisplayName = "Not Enough Mana"))
	void NotEnoughMana();

	UFUNCTION(BlueprintCallable, Category = "Controller", meta = (DisplayName = "Set Default Controller"))
	void SetDefaultController();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Camera", meta = (DisplayName = "Shake Cam"))
	void ShakeCam();

	// 레거시 SystemMessageComponent(BP): Play → HUD WB_Message 에 메시지, InTime 후 Hide. BP 변수명 과 겹침 회피.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components",
		meta = (DisplayName = "System Message Component"))
	TObjectPtr<UCSystemMessageComponent> NativeBpSystemMessageComp;

	// BP_Player::SetZooming — 블루프린트 줌 그래프가 캐릭터를 타깃으로 호출. CamComponent 와 동기화.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetZooming(double InZooming);

	// BP_Player::SaveZooming — OriginZooming 에 현재 Zooming 저장.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SaveZooming();

	// BP_Player::SetSkillZooming — Zooming 을 SkillZooming 값으로 설정.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetSkillZooming();

	// BP_Player::ApplyZoom — TargetArmLength 를 InZoom 쪽으로 Interp(ZoomData.InterpSpeed). 거의 같으면 생략.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void ApplyZoom(double InZoom);

	// BP_Player 점프 전 위치 저장(에어 무브·파쿠르).
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetCoordBeforeAir();

	/** `FlyToFall` 은 UHT/protected 레이아웃을 위해 보호 멤버 — 무기 에어 분기 등은 이 세터만 사용한다. */
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void SetFlyToFall(bool bFlyToFall) { FlyToFall = bFlyToFall; }

	// BP_Player::StartFall — 중력 스케일·낙하 모드·상태 Fall·FlyToFall (레거시 MovingComponent.SetGravity 그래프).
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void StartFall(double InGravity);

	// BP_Player::IsChangedLandCoord — 위치와 CoordBeforeAir 를 축별 허용 50 으로 비교 후 OR (출력 핀 ALittleMoved).
	UFUNCTION(BlueprintCallable, Category = "Movement", meta = (ReturnDisplayName = "ALittleMoved"))
	bool IsChangedLandCoord() const;

	// BP_Player::Tick_AirBone — 비행이 아니고 낙하 중일 때 FlyToFall 1회만 StartFall 유도.
	virtual void Tick_AirBone() override;

	// BP_Player::Tick_LerpMove — MovingComponent Lerp 플래그일 때 Dest 까지 보간, XY 도착 시 Lerp 종료.
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void Tick_LerpMove(float DeltaTime);

	// BP_Player::Tick_CheckGround — 액터 발밑 TraceTypeQuery1 라인트레이스 300u, 히트 시 true.
	UFUNCTION(BlueprintCallable, Category = "Movement|Ground")
	bool Tick_CheckGround() const;

	// BP_Player::Tick_AccelGravity — 바닥 트레이스 실패 시 중력 스케일 3 (MovingComponent).
	UFUNCTION(BlueprintCallable, Category = "Movement|Ground")
	void Tick_AccelGravity();

	// BP_Player::SetInvisibleMotionTrail / SetVisibleMotionTrail — 현재 Visible 과 반대일 때만 토글 (중복 Set 방지).
	UFUNCTION(BlueprintCallable, Category = "Trail")
	void SetInvisibleMotionTrail();

	UFUNCTION(BlueprintCallable, Category = "Trail")
	void SetVisibleMotionTrail();

	// BP_Player::Begin_SkillCam / End_SkillCam — 줌·뷰 타깃 블렌드, SkillSequence 재생(ActorSequence 플러그인 컴포넌트면 SequencePlayer::Play).
	UFUNCTION(BlueprintCallable, Category = "Camera|SkillCam")
	void Begin_SkillCam();

	UFUNCTION(BlueprintCallable, Category = "Camera|SkillCam")
	void End_SkillCam();

	// BP_Player::ReportNoise — 이동 속도(맥스 워크)가 MovingComponent 워크보다 클 때만 청각 이벤트.
	UFUNCTION(BlueprintCallable, Category = "AI")
	void ReportNoise();

	// BP_Player::SetFluidSim — 월드에서 FluidSimClass 검색 후 첫 액터를 FluidSimFolowing 에 넣고, Water/BP 전용 등록은 네이티브 이벤트로 확장.
	UFUNCTION(BlueprintCallable, Category = "Water")
	void SetFluidSim();

	// BP_Player::Tick_Fluid — FluidSimFolowing 이 유효하면 플레이어 위치로 이동.
	UFUNCTION(BlueprintCallable, Category = "Water")
	void Tick_Fluid();

	// FluidForceDynamic/Register Dynamic Force 는 Water UDS·BP 전용이라 기본 구현은 비우고 BP 에서 오버라이드한다.
	UFUNCTION(BlueprintNativeEvent, Category = "Water")
	void OnFluidSimActorRegistered(AActor* InFluidSimActor);

	/** YJJ_PlayerWeaponComp 우선 — BP 중복 WeaponComp·입력 바인딩 대상 통일. */
	UCWeaponComponent* EnsureWeaponComp();

	UFUNCTION()
	void EquipWeaponFromUI(const CEWeaponType InNewType);

	UFUNCTION()
	void EquipMagicFromUI(CEMagicType InNewType);

	UFUNCTION()
	void OnEquipMenuWeaponHoveredBridge(const CEWeaponType InType);

	UFUNCTION()
	void OnEquipMenuWeaponUnhoveredBridge(const CEWeaponType InType);

	UFUNCTION(BlueprintNativeEvent, Category = "UI|Menu", meta = (DisplayName = "Hovered Equip Menu (Weapon)"))
	void OnEquipMenuWeaponHovered(CEWeaponType InType);

	UFUNCTION(BlueprintNativeEvent, Category = "UI|Menu", meta = (DisplayName = "Unhovered Equip Menu (Weapon)"))
	void OnEquipMenuWeaponUnhovered(CEWeaponType InType);

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

	// UHT: BlueprintReadOnly/Write 는 private 에 둘 수 없음.
protected:
	// 레거시 BP_Player 컴포넌트 변수명 과 SKEL 속성 이름 충돌 방지: 네이티브 속성 접두 NativeBp-/Native.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "SpringArm"))
	TObjectPtr<USpringArmComponent> NativeSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (DisplayName = "Camera"))
	TObjectPtr<UCameraComponent> NativeCamera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCWeaponComponent> WeaponComp;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Components",
		meta = (DisplayName = "Weapon Component"))
	TObjectPtr<UCWeaponComponent> NativeBpWeaponAlias;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCMagicComponent> MagicComp;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Components",
		meta = (DisplayName = "Magic Component"))
	TObjectPtr<UCMagicComponent> NativeBpMagicAlias;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCTargetingComponent> TargetingComp;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Components",
		meta = (DisplayName = "Targeting Component"))
	TObjectPtr<UCTargetingComponent> NativeBpTargetingAlias;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCCamComponent> CamComp;

	// BP_Player 줌 — native FZoomData(CCamComponent) 사용. UserDefinedStruct FZoomData 대신 동일 필드로 블루프린트 핀을 맞춘다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (MultiLine = "true"))
	FZoomData ZoomData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (MultiLine = "true"))
	double Zooming = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (MultiLine = "true"))
	double OriginZooming = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (MultiLine = "true"))
	double SkillZooming = 250.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC", meta = (MultiLine = "true"))
	bool Jumping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC", meta = (MultiLine = "true"))
	FVector CoordBeforeAir = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC", meta = (MultiLine = "true"))
	double AirDistance = 500.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CC", meta = (MultiLine = "true"))
	bool FlyToFall = false;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCGameUIComponent> GameUIComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCInventoryComponent> InventoryComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCPlacementComponent> PlacementComp;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCParkourComponent> ParkourComp;

	UPROPERTY(EditAnywhere, Category = "Mode")
	TWeakObjectPtr<ACGameMode> GameMode;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	FVector2D PitchRange = FVector2D(-40, +40);
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	TArray<float> Speeds{ 200, 500, 800 };

	// BP_Player::ShakeCam — CS_NotEnoughState (블루프린트 기본값과 동일 경로로 생성자에서 채움).
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<UCameraShakeBase> NotEnoughStateCameraShakeClass;

	// BP SetColor 가 K2_SetTimer 로 연기하는 원색 복구.
	FTimerHandle BodyColorRestoreTimerHandle;

private:
	CEHitType CurHitType = CEHitType::Common;

	UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCUserWidget_EquipMenu> MenuEquipWidget;

	UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCUserWidget_MagicMenu> MenuMagicWidget;

	UPROPERTY(BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCUserWidget_Interaction> MenuInteractionWidget;
};