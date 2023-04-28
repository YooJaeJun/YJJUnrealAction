#include "GameMode/CGameMode.h"
#include "Global.h"

ACGameMode::ACGameMode()
{
	CHelpers::GetClass<APawn>(&DefaultPawnClass, "Blueprint'/Game/Player/BP_CPlayableCharacter.BP_CPlayableCharacter_C'");
}
