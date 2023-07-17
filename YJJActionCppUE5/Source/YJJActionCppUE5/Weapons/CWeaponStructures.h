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


USTRUCT()
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
	UPROPERTY(EditAnywhere)
		TObjectPtr<UAnimMontage> Montage;

	UPROPERTY(EditAnywhere)
		float PlayRate = 1;

	UPROPERTY(EditAnywhere)
		float Power;

	UPROPERTY(EditAnywhere)
		float Launch = 100;

	UPROPERTY(EditAnywhere)
		float StopTime;

	UPROPERTY(EditAnywhere)
		TObjectPtr<USoundWave> Sound;
	
	UPROPERTY(EditAnywhere)
		TObjectPtr<UFXSystemAsset> Effect;

	UPROPERTY(EditAnywhere)
		FVector EffectLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
		FVector EffectScale = FVector::OneVector;

	UPROPERTY(EditAnywhere)
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