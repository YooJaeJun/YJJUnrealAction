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
class ACAnimalWeapon;
class ACDragon;
class APawn;
class UCStateComponent;
class UCPatrolComponent;
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
	UFUNCTION(BlueprintPure, Category = "Character", meta = (DisplayName = "Is Same Group"))
	static bool BFCompat_IsSameGroup(ACCommonCharacter* InCharacterA, ACCommonCharacter* InCharacterB);

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

	// 레거시 BTTask_Animal_Action 의 GetComponentByClass → State 를 통해 IsIdle 을 거는 경로 노드 깨짐 방지(APawn 에서 호출 가능).
	UFUNCTION(BlueprintPure, Category = "AI|Behaviour Tree", meta = (DisplayName = "Is Idle (Controlled Pawn)"))
	static bool BFCompat_IsIdleFromControlledPawn(APawn* ControlledPawn);

	// 레거시 AnimalWeapon.BP 의 Get InAction 노드 깨짐 시 대체(ACAnimalWeapon::InAction).
	UFUNCTION(BlueprintPure, Category = "Animal|Weapon", meta = (DisplayName = "Animal Weapon — In Action"))
	static bool BFCompat_GetAnimalWeaponInAction(ACAnimalWeapon* AnimalWeapon);

	// BTTask_Animal_Action 등 — 제거된 BP_Animal_AI.Weapon 접근 회피(ACAnimal 의 AnimalWeapon, Display Name "Weapon").
	UFUNCTION(
		BlueprintPure,
		Category = "AI|Behaviour Tree",
		meta = (DisplayName = "Get Animal Weapon (From Controlled Pawn)"))
	static ACAnimalWeapon* BFCompat_GetAnimalWeaponFromControlledPawn(APawn* ControlledPawn);

	// BTService_DragonBoss / 레거시 BT — 제거된 BP_Dragon_AI 캐스트 대신 ACDragon (과거 로직 단일 클래스).
	UFUNCTION(BlueprintPure, Category = "AI|Behaviour Tree", meta = (DisplayName = "Get Controlled Dragon"))
	static ACDragon* BFCompat_GetControlledDragon(APawn* ControlledPawn);

	// BTService 드래곤 보스 등 — 무기 블프 InAction 과 동물 무기 헬퍼 공유(DragonWeapon 은 TObjectPtr<AActor>, ACAnimalWeapon 캐스트).
	UFUNCTION(BlueprintPure, Category = "AI|Behaviour Tree", meta = (DisplayName = "Dragon Weapon — In Action"))
	static bool BFCompat_GetDragonWeaponInAction(ACDragon* Dragon);

	/** BTService_DragonBoss::InAction — 드래곤 전용 무기 + 동물 무기 레이어 OR. */
	UFUNCTION(
		BlueprintPure,
		Category = "AI|Behaviour Tree",
		meta = (DisplayName = "DragonBoss — Weapons In Action (Pawn)"))
	static bool BFCompat_IsDragonBossWeaponsOrAnimalInAction(APawn* ControlledPawn);

	// UObject 로 깨진 Dragon 참조 없이 상태 조회 가능 — 레거시 GetComponent(State) 경로 교체 후 Type 핀은 UCStateComponent::Type 사용.
	UFUNCTION(BlueprintPure, Category = "AI|Behaviour Tree", meta = (DisplayName = "Find State Component (Pawn)"))
	static UCStateComponent* BFCompat_FindStateComponentOnPawn(APawn* ControlledPawn);

	// BTService_DragonBoss — 깨진 Get(State)→Type 또는 UObject Dragon 기반 GetComponentByClass 대신 공통 문자 CEStateType 으로 읽는다(블프 CurState 변수를 UDS EStateType 대신 CEStateType 로 바꾸는 것을 권장).
	UFUNCTION(
		BlueprintPure,
		Category = "AI|Behaviour Tree",
		meta = (DisplayName = "State — Get Type (Controlled Pawn)"))
	static CEStateType BFCompat_GetStateTypeFromControlledPawn(APawn* ControlledPawn);

	// 같은 서비스 — SetBehaviorType(InCharacter) 에 BP Dragon 캐시 없이 Pawn 에서 바로 긁을 때(ACDragon 포함).
	UFUNCTION(
		BlueprintPure,
		Category = "AI|Behaviour Tree",
		meta = (DisplayName = "Get Common Character (Controlled Pawn)"))
	static ACCommonCharacter* BFCompat_GetCommonCharacterFromControlledPawn(APawn* ControlledPawn);

	// 레거시 BTTask_Patrol — PatrolComponent_C 의 GetMoveTo / UpdateNextIndex 가 C++ UCPatrolComponent 로 옮겨졌을 때 블프 노드 유령화 방지.
	UFUNCTION(BlueprintPure, Category = "AI|Behaviour Tree", meta = (DisplayName = "Find Patrol Component (Pawn)"))
	static UCPatrolComponent* BFCompat_FindPatrolComponentOnPawn(APawn* ControlledPawn);

	UFUNCTION(BlueprintPure, Category = "AI|Behaviour Tree", meta = (DisplayName = "Patrol Get Move To (Pawn)"))
	static void BFCompat_PatrolGetMoveTo(APawn* ControlledPawn, bool& OutResult, FVector& OutLocation);

	UFUNCTION(BlueprintCallable, Category = "AI|Behaviour Tree", meta = (DisplayName = "Patrol Update Next Index (Pawn)"))
	static void BFCompat_PatrolUpdateNextIndex(APawn* ControlledPawn);

	/** Patrol Path 액터가 설정돼 있는지(UCPatrolComponent::IsValidPath) — BT 분기용 */
	UFUNCTION(BlueprintPure, Category = "AI|Behaviour Tree", meta = (DisplayName = "Patrol Is Valid Path (Pawn)"))
	static bool BFCompat_PatrolIsValidPath(APawn* ControlledPawn);

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
