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
		CEStateType StateType;

	UPROPERTY(EditAnywhere)
		CEHitType HitType;

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
		void PlayAnimMontage(const CEStateType InType);

	UFUNCTION(BlueprintCallable)
		void PlayHitAnimMontage(const CEHitType InType);

	void PlayAnimMontage(const CEStateType InType, const TWeakObjectPtr<UInputComponent> InInput) const;

private:
	UPROPERTY(EditAnywhere, Category = "DataTable")
		UDataTable* DataTable;

	UPROPERTY(EditAnywhere, Category = "DataTable")
		FMontagesData Datas[static_cast<uint8>(CEStateType::Max)];

	UPROPERTY(EditAnywhere, Category = "DataTable")
		FMontagesData HitDatas[static_cast<uint8>(CEHitType::Max)];

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
