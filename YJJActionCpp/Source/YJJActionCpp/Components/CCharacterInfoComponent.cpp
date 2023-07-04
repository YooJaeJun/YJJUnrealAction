#include "Components/CCharacterInfoComponent.h"
#include "Global.h"
#include "Commons/CGameInstance.h"

UCCharacterInfoComponent::UCCharacterInfoComponent()
{
	bWantsInitializeComponent = true;

	CurType = 0;
	CurName = "Name_None";
	CurGroup = 0;
	CurBodyColor = FLinearColor(0, 0, 0);
}

void UCCharacterInfoComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UCCharacterInfoComponent::IsSameGroup(TWeakObjectPtr<ACCommonCharacter> InOther) const
{
	const TWeakObjectPtr<UCCharacterInfoComponent> characterInfoComp = 
		CHelpers::GetComponent<UCCharacterInfoComponent>(InOther.Get());
	CheckNullResult(characterInfoComp, false);

	return GetCharacterGroup() == characterInfoComp->GetCharacterGroup();
}

void UCCharacterInfoComponent::SetCharacterType(const ECharacterType InNewType)
{
	const int32 newType = static_cast<int32>(InNewType);

	const TWeakObjectPtr<UCGameInstance> gameInst = Cast<UCGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	CheckNull(gameInst);

	CurInfo = *(gameInst->GetInfo(newType));
	CheckRefNullLog(CurInfo, "CurInfo doesn't exist.");

	CurType = newType;
	SetCharacterGroup(CurInfo.Group);
	SetCharacterName(CurInfo.Name);
	SetBodyColor(CurInfo.BodyColor);
}