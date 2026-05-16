#include "Utilities/YJJLocalizedText.h"
#include "Internationalization/TextKey.h"

namespace
{
	const FName GKey_MountInteract(TEXT("MountInteract"));
	const FName GKey_PlayerSpawnedNotice(TEXT("PlayerSpawnedNotice"));
	const FName GKey_NotEnoughStamina(TEXT("NotEnoughStamina"));
	const FName GKey_NotEnoughMana(TEXT("NotEnoughMana"));
}

const FName YJJLocalization::UserInterface_Localization_StringTableIdentifier(TEXT("ST_YJJ_UI"));

FText YJJLocalization::LocalizedText_From_UserInterface_Localization_Table_Key(FName Key)
{
	const FString keyString(Key.ToString());
	return FText::FromStringTable(UserInterface_Localization_StringTableIdentifier, FTextKey(keyString));
}

FText YJJLocalization::LocalizedText_MountInteract()
{
	return LocalizedText_From_UserInterface_Localization_Table_Key(GKey_MountInteract);
}

FText YJJLocalization::LocalizedText_PlayerSpawnedNotice()
{
	return LocalizedText_From_UserInterface_Localization_Table_Key(GKey_PlayerSpawnedNotice);
}

FText YJJLocalization::LocalizedText_NotEnough_Stamina()
{
	return LocalizedText_From_UserInterface_Localization_Table_Key(GKey_NotEnoughStamina);
}

FText YJJLocalization::LocalizedText_NotEnough_Mana()
{
	return LocalizedText_From_UserInterface_Localization_Table_Key(GKey_NotEnoughMana);
}
