#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CWeaponStructures.generated.h"

class UAnimMontage;
class UFXSystemAsset;
class ACCommonCharacter;
class USoundWave;

UENUM()
enum class EAttackType
{
	None,
	Knockback,
	Air,
	Fly,
	FlyingPutDown,
	Down,
	Max
};

USTRUCT()
struct FEquipmentData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		UAnimMontage* Montage;

	UPROPERTY(EditAnywhere)
		float PlayRate = 1;

	UPROPERTY(EditAnywhere)
		bool bCanMove = true;

	UPROPERTY(EditAnywhere)
		bool bUseControlRotation = true;

	UPROPERTY(EditAnywhere)
		TArray<USoundWave*> Sounds;

public:
	void PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner);
};


USTRUCT()
struct FActData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		UAnimMontage* Montage;

	UPROPERTY(EditAnywhere)
		float PlayRate = 1;

	UPROPERTY(EditAnywhere)
		bool bCanMove = true;

	UPROPERTY(EditAnywhere)
		bool bUseControlRotation = true;

	UPROPERTY(EditAnywhere)
		bool bFixedCamera;

	UPROPERTY(EditAnywhere)
		TArray<USoundWave*> Sounds;

	UPROPERTY(EditAnywhere)
		UFXSystemAsset* Effect;

	UPROPERTY(EditAnywhere)
		FVector EffectLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
		FVector EffectScale = FVector::OneVector;

public:
	void Act(const TWeakObjectPtr<ACCommonCharacter> InOwner);
	void PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner);
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation);
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation);
};


USTRUCT()
struct FHitData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		UAnimMontage* Montage;

	UPROPERTY(EditAnywhere)
		float PlayRate = 1;

	UPROPERTY(EditAnywhere)
		float Power;

	UPROPERTY(EditAnywhere)
		float Launch = 100;

	UPROPERTY(EditAnywhere)
		float StopTime;

	UPROPERTY(EditAnywhere)
		TArray<USoundWave*> Sounds;

	UPROPERTY(EditAnywhere)
		UFXSystemAsset* Effect;

	UPROPERTY(EditAnywhere)
		FVector EffectLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
		FVector EffectScale = FVector::OneVector;

	UPROPERTY(EditAnywhere)
		EAttackType AttackType = EAttackType::None;

public:
	void SendDamage(const TWeakObjectPtr<ACCommonCharacter> InAttacker, const TWeakObjectPtr<AActor> InAttackCauser,
		const TWeakObjectPtr<ACCommonCharacter> InOther);
	void PlayMontage(const TWeakObjectPtr<ACCommonCharacter> InOwner);
	void PlayHitStop(const TWeakObjectPtr<UWorld> InWorld);
	void PlaySoundWave(const TWeakObjectPtr<ACCommonCharacter> InOwner);
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation);
	void PlayEffect(const TWeakObjectPtr<UWorld> InWorld, const FVector& InLocation, const FRotator& InRotation);
};

USTRUCT()
struct FActDamageEvent
	: public FDamageEvent
{
	GENERATED_BODY()

public:
	FHitData* HitData;
};

UCLASS()
class YJJACTIONCPP_API UCWeaponStructures : public UObject
{
	GENERATED_BODY()
};