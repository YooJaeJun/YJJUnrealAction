#include "Components/CMontagesComponent.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "GameFramework/Character.h"
#include "Commons/CEnums.h"

UCMontagesComponent::UCMontagesComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());

	YJJHelpers::GetAsset<UDataTable>(&DataTable, "DataTable'/Game/Character/CDT_HumanAnim.CDT_HumanAnim'");
}

void UCMontagesComponent::BeginPlay()
{
	Super::BeginPlay();

	if (nullptr == DataTable)
	{
		CLog::Log("DataTable is not selected");
		return;
	}

	TArray<FMontagesData*> datas;
	DataTable->GetAllRows<FMontagesData>("", datas);

	constexpr uint32 size = static_cast<int32>(CEStateType::Max);

	for (uint32 i=0; i<size; i++)
	{
		for (const FMontagesData* data : datas)
		{
			if (data->StateType == static_cast<CEStateType>(i))
			{
				Datas[i] = *data;
				continue;
			}

			if (data->HitType == static_cast<CEHitType>(i))
			{
				HitDatas[i] = *data;
				continue;
			}
		}//for(data)
	}//for(i)
}


void UCMontagesComponent::PlayAvoidAnim()
{
	const TWeakObjectPtr<UInputComponent> input = YJJHelpers::GetComponent<UInputComponent>(Owner.Get());

	PlayAnimMontage(CEStateType::Avoid, input);
}

void UCMontagesComponent::PlayRiseAnim()
{
	PlayAnimMontage(CEStateType::Rise);
}

void UCMontagesComponent::PlayLandAnim()
{
	PlayAnimMontage(CEStateType::Land);
}

void UCMontagesComponent::PlayDeadAnim()
{
	PlayAnimMontage(CEStateType::Dead);
}

void UCMontagesComponent::PlayHitCommonAnim()
{
	PlayHitAnimMontage(CEHitType::Common);
}

void UCMontagesComponent::PlayHitKnockbackAnim()
{
	PlayHitAnimMontage(CEHitType::Knockback);
}

void UCMontagesComponent::PlayAnimMontage(const CEStateType InType)
{
	CheckNull(Owner);

	const FMontagesData data = Datas[static_cast<uint8>(InType)];

	if (nullptr == data.Montage)
	{
		CLog::Log("No Montages Data");
		return;
	}

	Owner->PlayAnimMontage(data.Montage, data.PlayRate);
}

void UCMontagesComponent::PlayHitAnimMontage(const CEHitType InType)
{
	CheckNull(Owner);

	const FMontagesData data = HitDatas[static_cast<uint8>(InType)];

	if (nullptr == data.Montage)
	{
		CLog::Log("No Hit Montages Data");
		return;
	}

	Owner->PlayAnimMontage(data.Montage, data.PlayRate);
}

void UCMontagesComponent::PlayAnimMontage(const CEStateType InType, const TWeakObjectPtr<UInputComponent> InInput) const
{
	CheckNull(Owner);

	const FMontagesData data = Datas[static_cast<uint8>(InType)];

	if (nullptr == data.Montage)
	{
		CLog::Log("No Montages Data");
		return;
	}

	Owner->SetActorRotation(Owner->MyCurController->GetControlRotation());

	FName startSectionName;

	if (InInput->GetAxisValue("MoveRight") > 0)
		startSectionName = "Right";
	else if (InInput->GetAxisValue("MoveRight") < 0)
		startSectionName = "Left";
	else if (InInput->GetAxisValue("MoveForward") > 0)
		startSectionName = "Front";
	else
		startSectionName = "Back";

	Owner->PlayAnimMontage(data.Montage, data.PlayRate, startSectionName);
}
