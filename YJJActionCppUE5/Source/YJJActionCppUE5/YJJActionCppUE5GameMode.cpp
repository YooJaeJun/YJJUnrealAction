#include "YJJActionCppUE5GameMode.h"
#include "YJJActionCppUE5Character.h"
#include "UObject/ConstructorHelpers.h"

AYJJActionCppUE5GameMode::AYJJActionCppUE5GameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
