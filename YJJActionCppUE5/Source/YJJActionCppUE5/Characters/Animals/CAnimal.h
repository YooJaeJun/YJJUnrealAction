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
#include "CAnimal.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnimalRidingSimpleEvent);

class UWidgetComponent;

// BP_Character 상속 BP_Animal 포팅 — 스탯 권위는 CharacterStatComp 가 우선일 수 있으나,
// BP 그래프·직렬화 호환을 위해 레거시 멤버를 둔다. (값 동기화는 BeginPlay 이후 설계에 따름.)
UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACAnimal : public ACCommonCharacter
{
	GENERATED_BODY()

public:
	ACAnimal();

protected:
	virtual void BeginPlay() override;

	// BP_Animal 의 HpBarWidget(WidgetComponent) — AI 는 오버라이드. 없으면 SetHpUI 는 HpBar_NPC 캐시만 갱신한다.
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

	// BP End_Dead 직후 스폰하던 레거시 파티클(예: P_SoulAura). 없으면 생략.
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

	// BP_Animal / BP_Player 와 동일한 발소리·착지 이펙트 배열.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "피직스|Animal", meta = (MultiLine = "true"))
	TArray<TObjectPtr<USoundBase>> FootstepSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "피직스|Animal", meta = (MultiLine = "true"))
	TArray<TObjectPtr<UNiagaraSystem>> FootstepEffects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "피직스|Animal", meta = (MultiLine = "true"))
	TArray<TObjectPtr<UNiagaraSystem>> LandEffects;

	// BP_Animal BeginPlay — CharacterStatComp 기준으로 Hp/MaxHp 미러 후 바 갱신.
	UFUNCTION(BlueprintCallable, Category = "Status|Animal", meta = (DisplayName = "Set Hp"))
	void SetHp();

	UFUNCTION(BlueprintCallable, Category = "UI|Animal", meta = (DisplayName = "Set Hp UI"))
	void SetHpUI();

	UFUNCTION(BlueprintCallable, Category = "UI|Animal", meta = (DisplayName = "Update Hp NPC"))
	void UpdateHp_NPC();

	// BP_Animal 상태 스위치 — 피격 연출(레거시 커스텀 이벤트 Hitted).
	UFUNCTION(BlueprintCallable, Category = "Combat|Animal", meta = (DisplayName = "Hitted"))
	void Hitted();

	// BP 노드명 Begin_Dead — Dead() 진입 래퍼.
	UFUNCTION(BlueprintCallable, Category = "Combat|Animal", meta = (DisplayName = "Begin Dead"))
	void Begin_Dead();

	// 애님 노티파이 / BP 가 I_Character 시그니처 대신 호출.
	UFUNCTION(BlueprintCallable, Category = "세팅|Animal")
	void FootstepAt(bool bLeftOrRight, EPhysicalSurface SurfaceType, FVector StepLocation);

	UFUNCTION(BlueprintCallable, Category = "세팅|Animal")
	void LandAt(EPhysicalSurface SurfaceType, FVector StepLocation);

	// BP_Player::SetDamage 와 동일 시맨틱 — InDamage<=0 이면 HitData.Damage 사용.
	UFUNCTION(BlueprintCallable, Category = "Combat|Animal")
	void SetDamage(float InDamage, bool& OutHittedOrDead);

	// BP 그래프 함수명 LevelUp — 레벨++ 후 Exp 에서 MaxExp 차감.
	UFUNCTION(BlueprintCallable, Category = "Reward|Animal", meta = (DisplayName = "Level Up"))
	void LevelUp();

	UFUNCTION(BlueprintCallable, Category = "Combat|Animal", meta = (DisplayName = "Load Prev State"))
	void LoadPrevState();

	// BP_Character::PlayHitAnim 근사 — HitData.Montage 우선, 없으면 HitAnim.
	UFUNCTION(BlueprintCallable, Category = "Combat|Animal", meta = (DisplayName = "Play Hit Anim"))
	void PlayHitAnim();

	UFUNCTION(BlueprintCallable, Category = "Hit|Animal", meta = (DisplayName = "Play Hit Unique Sound"))
	void PlayHitUniqueSound();

	UFUNCTION(BlueprintCallable, Category = "Hit|Animal", meta = (DisplayName = "Spawn Blood"))
	void SpawnBlood();

	// BP_Animal::PlayHitStop — HitStop 이 0 이 아닐 때만 적용. 피사체(폰 또는 Movable 메시)만 DilationActors 에 넣는다.
	UFUNCTION(BlueprintCallable, Category = "Hit|Animal", meta = (DisplayName = "Play Hit Stop"))
	void PlayHitStop();

	UFUNCTION(BlueprintCallable, Category = "Hit|Animal", meta = (DisplayName = "Restore Time Dilation"))
	void RestoreTimeDilation();

	// BP_PlayCameraShake — 공격자가 로컬 플레이어일 때만 HitData.ShakeClass 재생.
	virtual void PlayCameraShake() override;

	// BP Composite Reward — I_Reward.Rewarded 본문을 C++ 로 옮긴 진입점(그래프에서 직접 호출).
	UFUNCTION(BlueprintCallable, Category = "Reward|Animal")
	void ApplyRewardedEvent(ACharacter* Invoker, double InExp, int32 BuffIndex);

	UFUNCTION(BlueprintCallable, Category = "Reward|Animal")
	void LevelUpAnimal();

	// BP_Animal 인터페이스(AnimBP·레거시) 대응 — 세부 값은 프로젝트 애님에서 오버라이드.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animal|IK", meta = (DisplayName = "Toggle IK"))
	void ToggleIK();

	// BP_Animal::GetControlDirection — GetControlRotation 의 요만 사용(롤·피치 0) 후 전·우 단위 벡터.
	UFUNCTION(BlueprintCallable, Category = "Utilities", meta = (DisplayName = "Get Control Direction"))
	void GetControlDirection(FVector& OutForward, FVector& OutRight) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animal|Locomotion")
	FVector GetDesiredMovement();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Animal|IK")
	void SetFootLocation(bool bLeftFoot, FVector WorldLocation);

protected:
	virtual void Dead() override;
	virtual void End_Dead() override;

private:
	void TryGrantKillRewardToAttacker();

	UFUNCTION()
	void OnAnimalDeathAfterRewardDelay();

	UFUNCTION()
	void OnAnimalDeathSpawnSoulAndDestroy();

	bool bAnimalDeathSequenceStarted = false;

	FTimerHandle AnimalDeathTimer_RewardDelay;
	FTimerHandle AnimalDeathTimer_SoulFx;

protected:
	virtual void Footstep_Implementation() override;

	// HpBar_NPC 와(있으면) 자식의 HpBarWidget 위젯에 SetHPUI 브로드캐스트.
	virtual void RefreshAnimalHpBarWidgets();

	void ApplyEnemyHpBarPercent();

	float PlayLandMontageIfAny();

private:
	FTimerHandle HitStopRestoreTimer;
};
