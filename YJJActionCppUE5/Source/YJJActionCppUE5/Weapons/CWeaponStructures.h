#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Commons/CEnums.h"
#include "Engine/DamageEvents.h"
#include "CWeaponStructures.generated.h"

class UAnimMontage;
class UFXSystemAsset;
class ACCommonCharacter;
class UCCharacterStatComponent;
class USoundWave;

USTRUCT()
struct FEquipmentData
{
	GENERATED_BODY()

public:
	void PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere)
	float PlayRate = 1;

	UPROPERTY(EditAnywhere)
	bool bCanMove = true;

	UPROPERTY(EditAnywhere)
	bool bUseControlRotation = true;

	UPROPERTY(EditAnywhere)
	USoundWave* Sound;
};


USTRUCT()
struct FActData
{
	GENERATED_BODY()

public:
	void Act(const TWeakObjectPtr<ACCommonCharacter> InOwner) const;
	void PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner) const;
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation) const;
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation) const;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere)
	float PlayRate = 1;

	UPROPERTY(EditAnywhere)
	bool bCanMove = true;

	UPROPERTY(EditAnywhere)
	bool bUseControlRotation = true;

	UPROPERTY(EditAnywhere)
	bool bFixedCamera;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundWave> Sound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UFXSystemAsset> Effect;

	UPROPERTY(EditAnywhere)
	FVector EffectLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector EffectScale = FVector::OneVector;

	UPROPERTY(EditAnywhere)
	float Stamina;

	UPROPERTY(EditAnywhere)
	float Mana;
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Power = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Launch = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StopTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundWave* Sound = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UFXSystemAsset* Effect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EffectLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector EffectScale = FVector::OneVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	CEHitType AttackType = CEHitType::Common;
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