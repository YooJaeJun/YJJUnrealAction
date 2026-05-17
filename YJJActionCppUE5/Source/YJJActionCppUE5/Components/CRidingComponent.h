#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/CInterface_Interactable.h"
#include "Commons/CEnums.h"
#include "Components/CCamComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CRidingComponent.generated.h"

class USkeletalMeshComponent;
class UBoxComponent;
class ACCommonCharacter;
class ACGameMode;
class UCUserWidget_HUD;
class UCUserWidget_Interaction;
class ACAnimal;
class UTexture2D;
class USceneComponent;
class USpringArmComponent;
class UCameraComponent;
class UCCamComponent;
class UCMovementComponent;
class UCStateComponent;
class UAnimMontage;
class UCWeaponComponent;
class AController;
class USoundBase;
class UUserWidget;

UENUM()
enum class CERidingPoint : uint8
{
	CurMount,
	CandidateLeft,
	CandidateRight,
	CandidateBack,
	Rider,
	Unmount,
	Max
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class YJJACTIONCPPUE5_API UCRidingComponent :
	public UActorComponent,
	public ICInterface_Interactable
{
	GENERATED_BODY()

public:	
	UCRidingComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	void SetRidingState(const CERidingState InState);

	// InteractionCollision 델리게이트 — BP Custom Event 이름 BeginOverlap / EndOverlap 과 충돌하지 않게 분리.
	UFUNCTION()
	void MountInteraction_OnBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult);

	UFUNCTION()
	void MountInteraction_OnEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

public:
	void SetInteractor(
		TWeakObjectPtr<ACCommonCharacter> InCharacter, 
		const TWeakObjectPtr<ACCommonCharacter> InOtherCharacter);

	UFUNCTION()
	void SetRider(ACCommonCharacter* InCharacter);

	// ACCommonCharacter 가 전방 선언만 있을 때 헤더에서 TWeakObjectPtr(U*) 를 만들면 UObject 변환 static_assert 에 걸린다.
	TWeakObjectPtr<ACCommonCharacter> GetRider() const;

private:
	void Tick_MovingToMountPoint();
	void Tick_Mounting();
	void Tick_MountingEnd();
	void Tick_Riding() const;
	void Tick_Unmounting();
	void Tick_RidingEnd();

public:
	void CheckValidPoint();
	bool MoveToPoint(
		TWeakObjectPtr<ACCommonCharacter> Char, 
		const TWeakObjectPtr<USceneComponent> To);
	void PossessAndInterpToCamera();

	UFUNCTION()
	void InterpToRiderPos(UAnimMontage* Anim, bool bInterrupted);

	UFUNCTION()
	void AttachToRiderPoint(UAnimMontage* Anim, bool bInterrupted);

	void UnpossessAndInterpToCamera();

	UFUNCTION()
	void Unmount();

	// 심볼은 BP 컴포넌트 그래프의 Custom Event(Action, Menu …)와 이름 충돌을 피함. DisplayName 으로 에디터 표기만 맞춘다.
	UFUNCTION(BlueprintCallable, Category = "Riding|Input", meta = (DisplayName = "Action"))
	void Ride_Input_Action();

	void InputAction_Act();

	UFUNCTION(BlueprintCallable, Category = "Riding|Input", meta = (DisplayName = "SubWeapon Action Pressed"))
	void Ride_Input_SubWeaponPressed();

	UFUNCTION(BlueprintCallable, Category = "Riding|Input", meta = (DisplayName = "SubWeapon Action Released"))
	void Ride_Input_SubWeaponReleased();

	UFUNCTION(BlueprintCallable, Category = "Riding|Input", meta = (DisplayName = "Skill"))
	void Ride_Input_Skill(int32 InIndex);

	UFUNCTION(BlueprintCallable, Category = "Riding|Input", meta = (DisplayName = "Skill 1 Pressed"))
	void Ride_Input_Skill1Pressed();

	UFUNCTION(BlueprintCallable, Category = "Riding|Input", meta = (DisplayName = "Skill 2 Pressed"))
	void Ride_Input_Skill2Pressed();

	UFUNCTION(BlueprintCallable, Category = "Riding|Combat", meta = (DisplayName = "Cancel Hit Anim"))
	void CancelHitAnim();

	UFUNCTION(BlueprintCallable, Category = "Riding|Input", meta = (DisplayName = "Skill Unmount"))
	void Skill_Unmount();

	UFUNCTION(BlueprintCallable, Category = "Riding|Input", meta = (DisplayName = "Magic"))
	void Ride_Input_Magic();

	UFUNCTION(BlueprintCallable, Category = "Riding|Input")
	void ZoomInput(double InAxis);

	// 레거시 RidingComponent BP SetZooming — 줌 목표값을 CamComp 와 동기화한다.
	UFUNCTION(BlueprintCallable, Category = "Riding|Zoom", meta = (DisplayName = "Set Zooming"))
	void SetZooming(double InZooming);

	// 레거시 RidingComponent BP ApplyZoom — 스프링암 보간 후 CamComp.Zooming 과 맞춰 틱 보간과 싸우지 않게 한다.
	UFUNCTION(BlueprintCallable, Category = "Riding|Zoom", meta = (DisplayName = "Apply Zoom"))
	void ApplyZoom(double InZoom);

	UFUNCTION(BlueprintCallable, Category = "Riding|Input")
	void TargetingInput();

	// 레거시 IsNotBowMode 대체 — 라이더 무기가 활 모드인지.
	UFUNCTION(BlueprintPure, Category = "Riding|State")
	bool IsBowMode() const;

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Menu"))
	void Ride_Input_Menu();

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Menu Hide"))
	void Ride_Input_MenuHide();

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Magic Menu"))
	void Ride_Input_MagicMenu();

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Magic Menu Hide"))
	void Ride_Input_MagicMenuHide();

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Hovered Equip"))
	void Ride_Ui_HoveredEquip(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Unhovered Equip"))
	void Ride_Ui_UnhoveredEquip(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Clicked Equip"))
	void Ride_Ui_ClickedEquip(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Hovered Magic"))
	void Ride_Ui_HoveredMagic(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Unhovered Magic"))
	void Ride_Ui_UnhoveredMagic(const FString& InName);

	UFUNCTION(BlueprintCallable, Category = "Riding|UI", meta = (DisplayName = "Clicked Magic"))
	void Ride_Ui_ClickedMagic(const FString& InName);

	// ICInterface_Interactable / 레거시 I_Interactable::GetAction — InteractionText(탑승) 반환.
	UFUNCTION(BlueprintCallable, Category = "Riding|Interact", meta = (DisplayName = "Get Action"))
	virtual void GetAction(FText& OutText) const override;

	void Input_Zoom(const float InAxis) { ZoomInput(static_cast<double>(InAxis)); }
	void Input_Targeting() { TargetingInput(); }

	// ICInterface_Interactable — BP 에서 Interact 키로 탑승 해제할 때 호출.
	virtual void EndInteraction() override;

	// 블루프린트의 SetStatusUI / OnStatusUI 그래프 대체 — 동일 이름의 BP 함수가 있으면 에디터에서 제거한다(비가상 C++ 호출만 실행됨).
	void SetStatusUI();
	void OnStatusUI(const bool InOn);

	// --- BPVar (RidingComponent.RidingComponent_C) 이름·카테고리 정합 ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "세팅", meta = (DisplayName = "Owner"))
	TObjectPtr<ACAnimal> Owner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Mesh"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Interaction Collision"))
	TObjectPtr<UBoxComponent> InteractionCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Mount Left"))
	TObjectPtr<USceneComponent> MountLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Mount Right"))
	TObjectPtr<USceneComponent> MountRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Mount Back"))
	TObjectPtr<USceneComponent> MountBack;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Mount"))
	TObjectPtr<USceneComponent> Mount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Rider Point"))
	TObjectPtr<USceneComponent> RiderPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Unmount"))
	TObjectPtr<USceneComponent> UnmountPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Spring Arm"))
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Camera"))
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Moving"))
	TObjectPtr<UCMovementComponent> Moving;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Rider Weapon"))
	TObjectPtr<UCWeaponComponent> RiderWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Rider Magic"))
	TObjectPtr<UActorComponent> RiderMagic;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "세팅", meta = (DisplayName = "Rider"))
	TObjectPtr<ACCommonCharacter> Rider;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (DisplayName = "Hp Bar"))
	TObjectPtr<UUserWidget> HpBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (DisplayName = "Level Bar"))
	TObjectPtr<UUserWidget> LevelBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (DisplayName = "Height UI"))
	TObjectPtr<UUserWidget> HeightUI;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact", meta = (DisplayName = "Interactable Actor"))
	TObjectPtr<AActor> InteractableActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount", meta = (DisplayName = "Mount Anims"))
	TArray<TObjectPtr<UAnimMontage>> MountAnims;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mount", meta = (DisplayName = "Mount Anim"))
	TObjectPtr<UAnimMontage> MountAnim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount", meta = (DisplayName = "Mount Rotation ZFactor"))
	double MountRotationZFactor = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mount", meta = (DisplayName = "Mount Dir"))
	CEDirection MountDir = CEDirection::Left;

	UPROPERTY(EditDefaultsOnly, Category = "Mount", meta = (DisplayName = "Unmount Anim"))
	TObjectPtr<UAnimMontage> UnmountAnim;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mount", meta = (DisplayName = "Controller Save"))
	TObjectPtr<AController> ControllerSave;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (DisplayName = "Riding State"))
	CERidingState RidingState = CERidingState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State", meta = (DisplayName = "Flying Type"))
	bool FlyingType = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deco", meta = (DisplayName = "Eye Class"))
	TSubclassOf<AActor> EyeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deco", meta = (DisplayName = "Eye"))
	TObjectPtr<AActor> Eye;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (DisplayName = "Zooming"))
	double Zooming = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom", meta = (DisplayName = "Zoom Data"))
	FZoomData ZoomData;

	UPROPERTY(EditDefaultsOnly, Category = "IK", meta = (DisplayName = "Leg IKAlpha"))
	double LegIKAlpha = 0.2;

	// 블루프린트 Variable Get "StateComponent" 이름과 동일해야 그래프가 재연결된다. 탈것 쪽 UCStateComponent.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default", meta = (DisplayName = "State Component"))
	TObjectPtr<UCStateComponent> StateComponent;

private:
	UFUNCTION()
	void OnRidingAnimalHpUiRequested();

	UFUNCTION()
	void OnRidingAnimalExpUiRequested();

	UFUNCTION()
	void OnRidingAnimalLevelUiRequested();

	void BindRidingAnimalUiDelegates();
	void UnbindRidingAnimalUiDelegates();
	void RefreshRidingAnimalHudFromAnimal();
	void RefreshRidingHpBarWidgetFromAnimal();
	void RefreshRidingLevelBarWidgetFromAnimal();
	void TryPlayRidingLevelBarLevelUpEffect();

	UCUserWidget_HUD* ResolveLocalHud();

	void ApplyMountAnimsFromBlueprintArray();

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	TObjectPtr<UTexture2D> InteractionKeyTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	FText InteractionText;

	// CEDirection 인덱스별 탑승 몽타주 — MountAnims TArray(3) 또는 기본 에셋 로드로 채운다.
	TObjectPtr<UAnimMontage> MountAnimByDirection[static_cast<uint8>(CEDirection::Max)];

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	TObjectPtr<USoundBase> MountSound;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	float OverTime_Camera = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	float OverTime_Mount = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	float OverTime_RiderPos = 0.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	float OverTime_Unmount = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Mount")
	TObjectPtr<USoundBase> UnmountSound;

	TWeakObjectPtr<UCStateComponent> StateComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;
	TWeakObjectPtr<UCCamComponent> CamComp;
	TWeakObjectPtr<UCUserWidget_HUD> Hud;
	TWeakObjectPtr<UCStateComponent> RiderStateComp;
	TWeakObjectPtr<UCMovementComponent> RiderMovementComp;
	TWeakObjectPtr<UCCamComponent> RiderCamComp;
	TWeakObjectPtr<UCWeaponComponent> RiderWeaponComp;
	TWeakObjectPtr<USceneComponent> RidingPoints[static_cast<uint8>(CERidingPoint::Max)];
	TWeakObjectPtr<UCUserWidget_Interaction> Interaction;
	TEnumAsByte<EMoveComponentAction::Type> eMoveAction;
	FLatentActionInfo latentInfo;
};
