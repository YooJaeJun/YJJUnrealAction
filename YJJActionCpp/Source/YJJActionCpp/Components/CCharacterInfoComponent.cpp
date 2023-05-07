#include "Components/CCharacterInfoComponent.h"
#include "Global.h"
#include "Game/CGameInstance.h"

UCCharacterInfoComponent::UCCharacterInfoComponent()
{
	bWantsInitializeComponent = true;

	CurName = "¿Ã∏ß";
	CurGroupIndex = 0;
	CurBodyColor = FLinearColor(0, 0, 0);
}

void UCCharacterInfoComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UCCharacterInfoComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SetCharacterName(TEXT("Player"));
}

void UCCharacterInfoComponent::SetCharacterName(const FName InNewName)
{
	UCGameInstance* gameInst = Cast<UCGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));

	CheckNull(gameInst);

	CurInfo = gameInst->GetInfo(InNewName);

	if (nullptr != CurInfo)
	{
		CurName = InNewName;
		SetGroupIndex(CurInfo->GroupIndex);
		SetBodyColor(CurInfo->BodyColor);
	}
	else
		CLog::Log("New Name data doesn't exist.");
}

void UCCharacterInfoComponent::SetGroupIndex(int32 InNewGroupIndex)
{
	CurGroupIndex = InNewGroupIndex;
}

void UCCharacterInfoComponent::SetBodyColor(const FLinearColor& InNewColor)
{
	CurBodyColor = InNewColor;
}

const FName UCCharacterInfoComponent::GetCharacterName() const
{
	CheckNullResult(CurInfo, "CurInfo is nullptr");

	return CurInfo->Name;
}

int32 UCCharacterInfoComponent::GetGroupIndex() const
{
	CheckNullResult(CurInfo, -1);

	return CurInfo->GroupIndex;
}
