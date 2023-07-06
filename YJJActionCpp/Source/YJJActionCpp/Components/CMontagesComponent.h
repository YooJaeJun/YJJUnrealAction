#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/CStateComponent.h"
#include "Engine/DataTable.h"
#include "CMontagesComponent.generated.h"

class UAnimMontage;
class ACCommonCharacter;

USTRUCT()
struct FMontagesData
	: public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
		EStateType StateType;

	UPROPERTY(EditAnywhere)
		EHitType HitType;

	UPROPERTY(EditAnywhere)
		UAnimMontage* Montage;

	UPROPERTY(EditAnywhere)
		float PlayRate = 1;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCMontagesComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCMontagesComponent();

protected:
	virtual void BeginPlay() override;

public:
	void PlayAvoidAnim();
	void PlayRiseAnim();
	void PlayLandAnim();
	void PlayDeadAnim();

	void PlayHitCommonAnim();
	void PlayHitKnockbackAnim();

private:
	UFUNCTION(BlueprintCallable)
		void PlayAnimMontage(const EStateType InType);

	UFUNCTION(BlueprintCallable)
		void PlayHitAnimMontage(const EHitType InType);

	void PlayAnimMontage(const EStateType InType, const TWeakObjectPtr<UInputComponent> InInput) const;

private:
	UPROPERTY(EditAnywhere, Category = "DataTable")
		UDataTable* DataTable;

	UPROPERTY(EditAnywhere, Category = "DataTable")
		FMontagesData Datas[static_cast<uint8>(EStateType::Max)];

	UPROPERTY(EditAnywhere, Category = "DataTable")
		FMontagesData HitDatas[static_cast<uint8>(EHitType::Max)];

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
