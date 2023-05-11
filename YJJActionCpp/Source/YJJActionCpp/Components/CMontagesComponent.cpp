#include "Components/CMontagesComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "GameFramework/Character.h"

UCMontagesComponent::UCMontagesComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCMontagesComponent::BeginPlay()
{
	Super::BeginPlay();

	if (nullptr == DataTable)
	{
		GLog->Log(ELogVerbosity::Error, "DataTable is not selected");

		return;
	}

	TArray<FMontagesData*> datas;
	DataTable->GetAllRows<FMontagesData>("", datas);

	for (int32 i=0; i<static_cast<int32>(EStateType::Max); i++)
	{
		for (FMontagesData* data : datas)
		{
			if (static_cast<EStateType>(i) == data->StateType)
			{
				Datas[i] = *data;

				continue;
			}
		}//for(data)
	}//for(i)
}


void UCMontagesComponent::PlayAvoidAnim()
{
	const TWeakObjectPtr<UInputComponent> input = 
		Cast<UInputComponent>(Owner->GetComponentByClass(UInputComponent::StaticClass()));

	PlayAnimMontage(EStateType::Avoid, input);
}

void UCMontagesComponent::PlayRiseAnim()
{
	PlayAnimMontage(EStateType::Rise);
}

void UCMontagesComponent::PlayHitAnim()
{
	PlayAnimMontage(EStateType::Hit);
}

void UCMontagesComponent::PlayDeadAnim()
{
	PlayAnimMontage(EStateType::Dead);
}

void UCMontagesComponent::PlayAnimMontage(const EStateType InType)
{
	CheckNull(Owner);

	const FMontagesData data = Datas[(uint8)InType];

	if (nullptr == data.Montage)
	{
		GLog->Log(ELogVerbosity::Error, "No Montages Data");

		return;
	}

	Owner->PlayAnimMontage(data.Montage, data.PlayRate);
}

void UCMontagesComponent::PlayAnimMontage(const EStateType InType, const TWeakObjectPtr<UInputComponent> InInput)
{
	CheckNull(Owner);

	const FMontagesData data = Datas[(uint8)InType];

	if (nullptr == data.Montage)
	{
		GLog->Log(ELogVerbosity::Error, "No Montages Data");

		return;
	}

	if (InInput->GetAxisValue("MoveRight") > 0)
		Owner->PlayAnimMontage(data.Montage, data.PlayRate, "Right");
	else if (InInput->GetAxisValue("MoveRight") < 0)
		Owner->PlayAnimMontage(data.Montage, data.PlayRate, "Left");
	else if (InInput->GetAxisValue("MoveForward") > 0)
		Owner->PlayAnimMontage(data.Montage, data.PlayRate, "Front");
	else
		Owner->PlayAnimMontage(data.Montage, data.PlayRate, "Back");
}
