#pragma once

#include "CoreMinimal.h"
#include "Internationalization/Text.h"

// String Table 재임포트 CSV 는 1행을 `---,테이블ID` 형식으로 둔다. 두 번째 열(예: ST_YJJ_UI)이
// 코드의 UserInterface_Localization_StringTableIdentifier 및 FStringTable 레지스트리 Id 와 반드시 일치해야 한다.
// (컨텐츠 브라우저의 에셋 파일명이 Id 와 같은 경우가 많지만, 검증해야 할 것은 Id 문자열 일치이다.)
// 재임포트: ST_YJJ_UI 스트링 테이블 에셋 → Asset Details 에서 소스 파일로 이 프로젝트의 LocalizedText.csv 지정 후 Reimport.
namespace YJJLocalization
{
	extern const FName UserInterface_Localization_StringTableIdentifier;

	YJJACTIONCPPUE5_API FText LocalizedText_From_UserInterface_Localization_Table_Key(FName Key);

	YJJACTIONCPPUE5_API FText LocalizedText_MountInteract();
	YJJACTIONCPPUE5_API FText LocalizedText_PlayerSpawnedNotice();
	YJJACTIONCPPUE5_API FText LocalizedText_NotEnough_Stamina();
	YJJACTIONCPPUE5_API FText LocalizedText_NotEnough_Mana();
}
