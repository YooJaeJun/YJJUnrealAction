#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Commons/CGameInstance.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Weapons/CWeaponStructures.h"
#include "Camera/CameraShakeBase.h"
#include "CYJJBlueprintLibrary.generated.h"

class UAnimMontage;
class USoundWave;

// 블루프린트 전용 헬퍼. C++ 타입 FHitData 와 Content 의 UserDefinedStruct 이름이 겹치면 기본 Break 노드가 죽을 수 있어
// NativeBreak 로 분해 경로를 고정한다.
UCLASS()
class YJJACTIONCPPUE5_API UCYJJBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// FCCharacterInfo → 선행 F 제거 시 CCharacterInfo 이므로 NativeBreak 함수명은 BreakCCharacterInfo 이다.
	// Content 의 UserDefinedStruct "FCharacter Info" 등과 C++ 타입이 섞이면 핀 호환 오류가 난다 → UDS 제거 후 이 타입만 쓴다.
	UFUNCTION(BlueprintPure, meta = (NativeBreak), Category = "YJJ|Character")
	static void BreakCCharacterInfo(
		UPARAM(DisplayName = "Character Info") const FCCharacterInfo& CharacterInfo,
		int32& Type,
		int32& Group,
		FName& Name,
		FLinearColor& BodyColor);

	// String Table Identifier 는 YJJLocalization::UserInterface_Localization_StringTableIdentifier 과 동일. 키는 Source/YJJActionCppUE5/LocalizedText.csv .
	UFUNCTION(BlueprintPure, Category = "YJJ|Localization")
	static FText GetLocalizedUI(FName Key);

	UFUNCTION(BlueprintPure, meta = (NativeBreak), Category = "YJJ|Hit")
	static void BreakHitData(
		UPARAM(DisplayName = "Hit Data") const FHitData& HitData,
		UAnimMontage*& Montage,
		float& PlayRate,
		float& Power,
		float& Launch,
		float& StopTime,
		USoundWave*& Sound,
		UFXSystemAsset*& Effect,
		FVector& EffectLocation,
		FVector& EffectScale,
		CEHitType& AttackType,
		TSubclassOf<UCameraShakeBase>& ShakeClass);
};
