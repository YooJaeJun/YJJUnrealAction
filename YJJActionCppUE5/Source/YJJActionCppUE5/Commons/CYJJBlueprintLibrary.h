#pragma once

#include "CoreMinimal.h"
#include "Commons/CEnums.h"
#include "Commons/CGameInstance.h"
#include "Engine/EngineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Weapons/CWeaponStructures.h"
#include "Camera/CameraShakeBase.h"
#include "CYJJBlueprintLibrary.generated.h"

class AActor;
class ACharacter;
class ACCommonCharacter;
class UAnimMontage;
class UPrimitiveComponent;
class USoundBase;
class UTextRenderComponent;

// 블루프린트 전용 헬퍼. C++ 타입 FHitData 와 Content 의 UserDefinedStruct 이름이 겹치면 기본 Break 노드가 죽을 수 있어
// NativeBreak 로 분해 경로를 고정한다.
UCLASS()
class YJJACTIONCPPUE5_API UCYJJBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// FCCharacterInfo → 선행 F 제거 시 CCharacterInfo 이므로 NativeBreak 함수명은 BreakCCharacterInfo 이다.
	// Content 의 UserDefinedStruct "FCharacter Info" 등과 C++ 타입이 섞이면 핀 호환 오류가 난다 → UDS 제거 후 이 타입만 쓴다.
	UFUNCTION(BlueprintPure, meta = (NativeBreak), Category = "Character")
	static void BreakCCharacterInfo(
		UPARAM(DisplayName = "Character Info") const FCCharacterInfo& CharacterInfo,
		int32& Type,
		int32& Group,
		FName& Name,
		FLinearColor& BodyColor);

	// String Table Identifier 는 YJJLocalization::UserInterface_Localization_StringTableIdentifier 과 동일. 키는 Source/YJJActionCppUE5/LocalizedText.csv .
	UFUNCTION(BlueprintPure, Category = "Localization")
	static FText GetLocalizedUI(FName Key);

	// 레거시 BF_Helpers::IsSameGroup — Break FCharacterInfo UDS 노드와 C++ CharacterInfo 핀 타입 불일치로 BP 컴파일이 깨질 때 교체한다.
	UFUNCTION(BlueprintPure, Category = "Character", meta = (DisplayName = "Are Characters Same Group"))
	static bool AreCharactersSameGroup(ACCommonCharacter* InA, ACCommonCharacter* InB);

	UFUNCTION(
		BlueprintCallable,
		Category = "Character",
		meta = (DisplayName = "Find And Add Character (Unique)"))
	static bool TryAddUniqueCommonCharacter(
		TArray<ACCommonCharacter*>& InOutCharacters,
		ACCommonCharacter* ToAddCandidate);

	// 레거시 BF_Helpers::GetNearlyFrontAngle — 기준 Character 와 같은 컨트롤 회전 축 전방 벡터에 대해 후보 목록 중 front dot 이 가장 크고 Limit Minimum Dot 를 넘기는 하나를 고른다.
	UFUNCTION(BlueprintPure, Category = "AI", meta = (DisplayName = "Find Nearly Front Character"))
	static ACharacter* FindNearlyFrontCharacter(
		ACharacter* ReferenceCharacter,
		const TArray<ACharacter*>& CandidateCharacters,
		double LimitMinimumDot = 0.7);

	// 레거시 BF_Helpers::RotateFrom360To180 (ABP 회전 각 정규화 등).
	UFUNCTION(BlueprintPure, Category = "Math", meta = (DisplayName = "Rotate From 360 To 180"))
	static double RotateYawFrom360ToMinus180Degrees(double DegreesAroundFullCircle);

	UFUNCTION(
		BlueprintCallable,
		Category = "Rendering",
		meta = (DisplayName = "Set State Text (Render)"))
	static void SetStateRenderText(UTextRenderComponent* RenderComponent, const FText& DisplayText);

	// 레거시 BF_Helpers::GetHitPoint — SphereTraceMultiForObjects 기본 채널은 Pawn + WorldDynamic(레거시 ObjectTypeQuery3/4 대응 근사).
	UFUNCTION(BlueprintCallable, Category = "Collision", meta = (DisplayName = "Get Hit Point (Sphere Between)"))
	static bool TryGetHitPointBetweenPrimitives(
		UPrimitiveComponent* OverlapStartComponent,
		UPrimitiveComponent* OtherComponent,
		float SphereRadius,
		const TArray<AActor*>& ActorsToIgnore,
		FVector& OutImpactPoint);

	UFUNCTION(BlueprintPure, meta = (NativeBreak), Category = "Hit")
	static void BreakHitData(
		UPARAM(DisplayName = "Hit Data") const FHitData& HitData,
		UAnimMontage*& Montage,
		float& PlayRate,
		UPARAM(DisplayName = "Damage") float& Damage,
		CEAttackType& AttackType,
		bool& bCanMove,
		float& Launch,
		CECrowdControl& CrowdControl,
		UPARAM(DisplayName = "Hit Stop") float& HitStop,
		USoundBase*& Sound,
		UFXSystemAsset*& Effect,
		FVector& EffectLocation,
		FVector& EffectScale,
		TSubclassOf<UCameraShakeBase>& ShakeClass);
};
