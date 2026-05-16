#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Commons/CEnums.h"
#include "Engine/DamageEvents.h"
#include "Camera/CameraShakeBase.h"
#include "Sound/SoundBase.h"
#include "CWeaponStructures.generated.h"

class UAnimMontage;
class UFXSystemAsset;
class ACCommonCharacter;
class UCCharacterStatComponent;
class USoundWave;

// BP Structure "FEquipData" 대응.
USTRUCT(BlueprintType)
struct FEquipData
{
	GENERATED_BODY()

public:
	void PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseControlRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundWave* Sound;
};

// BP Structure "FDoActionData" 대응 — Tag·AttackType·모션트레일·런치는 BP 필드를 C++ 로 옮긴 것.
USTRUCT(BlueprintType)
struct FDoActionData
{
	GENERATED_BODY()

public:
	void Act(const TWeakObjectPtr<ACCommonCharacter> InOwner) const;
	void PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const;
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation) const;
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Tag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	CEAttackType AttackType = CEAttackType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseControlRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFixedCamera = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundWave> Sound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UFXSystemAsset> Effect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EffectLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EffectScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseMotionTrail = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stamina = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Mana = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchForward = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LaunchUp = 0.0f;
};

USTRUCT(BlueprintType)
struct FHitData
{
	GENERATED_BODY()

public:
	void SendDamage(const TWeakObjectPtr<ACCommonCharacter> InAttacker,
		const TWeakObjectPtr<AActor> InAttackCauser,
		const TWeakObjectPtr<ACCommonCharacter> InOther) const;
	void PlayMontage(const TWeakObjectPtr<ACCommonCharacter> InOwner) const;
	void PlayHitStop(const TWeakObjectPtr<UWorld> InWorld) const;
	void PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const;
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation) const;
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation) const;

public:
	// UObject 필드는 raw 포인터로 두면 USTRUCT Break/Make 와 에셋 직렬화가 안정적이다.
	// Content 에 UserDefinedStruct 로 FHitData 를 만들었다면 이름 충돌로 Break 가 깨지므로 UDS 는 삭제하거나 개명한다.
	// 그래도 실패하면 UCYJJBlueprintLibrary::BreakHitData(NativeBreak) 를 쓴다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlayRate = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Damage"))
	float Damage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	CEAttackType AttackType = CEAttackType::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Launch = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	CECrowdControl CrowdControl = CECrowdControl::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayName = "Hit Stop"))
	float HitStop = 0.0f;

	// BP FHitData 의 ShakeClass / 레거시 LegacyCameraShake — CameraShakeBase 파생이면 그대로 대입 가능.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UCameraShakeBase> ShakeClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* Sound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UFXSystemAsset* Effect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EffectLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EffectScale = FVector::OneVector;
};

USTRUCT()
struct FActDamageEvent
	: public FDamageEvent
{
	GENERATED_BODY()

public:
	FHitData HitData;
};

UCLASS()
class YJJACTIONCPPUE5_API UCWeaponStructures : public UObject
{
	GENERATED_BODY()
};
