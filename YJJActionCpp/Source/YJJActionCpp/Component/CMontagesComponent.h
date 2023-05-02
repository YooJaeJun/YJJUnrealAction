#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Component/CStateComponent.h"
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
		UAnimMontage* Montage;

	UPROPERTY(EditAnywhere)
		float PlayRate = 1;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCMontagesComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "DataTable")
		UDataTable* DataTable;

	UPROPERTY(EditAnywhere, Category = "DataTable")
		FMontagesData Datas[static_cast<uint8>(EStateType::Max)];

public:	
	UCMontagesComponent();

protected:
	virtual void BeginPlay() override;

public:
	void PlayAvoidAnim();

private:
	void PlayAnimMontage(const EStateType InType);

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
