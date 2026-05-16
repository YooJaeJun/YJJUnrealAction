#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Characters/CCharacterFeetData.h"
#include "CAnimalAnimDynamicInfo.generated.h"

class APawn;
class ACCommonCharacter;

// ABP_AnimalAnim_DynamicInfo 레거시 — AnimInstance(말 등)가 스폰해 매 틱 갱신 값을 넘기는 보조 액터.
UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API ACAnimalAnimDynamicInfo : public AActor
{
	GENERATED_BODY()

public:
	ACAnimalAnimDynamicInfo();

	// 레거시 ABP Tick_Info(BP_DynamicInfo 그래프)와 동등한 채우기: AnimInstance(Character) 값 + Movement Falling · State 의 실탑승 · UCFeetComponent IK 스칼라. 서브클래스는 여기 외 추가만.
	UFUNCTION(BlueprintNativeEvent, Category = "AnimalAnim")
	void Tick_Info(APawn* InOwner);
	virtual void Tick_Info_Implementation(APawn* InOwner);

	// BlueprintNativeEvent 로 생성된 Tick_Info 는 일반 블루프린트(AnimBP 등)에서 직접 호출 불가 — 호출 노드 호환용 Callable.
	UFUNCTION(BlueprintCallable, Category = "AnimalAnim", meta = (DisplayName = "Tick Info (BP 호출 전용)", ToolTip = "ABP 에서 레거시 Tick_Info 호출 노드를 이 함수로 교체하면 컴파일된다."))
	void BP_CallTick_Info(APawn* InOwner);

	// 레거시 BP_AnimalAnim_DynamicInfo 순수 함수(스테이트 머신 전환) — 부모 클래스 호출만 쓰이면 블루프린트에서 그래프 삭제 가능.
	UFUNCTION(BlueprintPure, Category = "AnimalAnim|Transitions", meta = (ReturnDisplayName = "Falling"))
	bool FromMoveToJumpStart() const;

	UFUNCTION(BlueprintPure, Category = "AnimalAnim|Transitions")
	bool FromFallingToMove() const;

	UFUNCTION(BlueprintPure, Category = "AnimalAnim|Transitions")
	bool FromMoveToFalling() const;

	UFUNCTION(BlueprintPure, Category = "AnimalAnim|Transitions")
	bool FromMoveToRotate() const;

	UFUNCTION(BlueprintPure, Category = "AnimalAnim|Transitions", meta = (ReturnDisplayName = "Falling"))
	bool FromRotateToJumpStart() const;

	// 이름 Character 는 UActorComponent::GetCharacter() 등과 헷갈릴 수 있으나 레거시 BPVar VarName 과 핀 이름 일치 우선.
	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Character"))
	TObjectPtr<ACCommonCharacter> Character;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Speed"))
	double Speed = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Direction"))
	double Direction = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Controller Direction"))
	double ControllerDirection = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Controller Direction 180"))
	double ControllerDirection180 = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Side"))
	double Side = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Forward"))
	double Forward = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Prev Rotation"))
	FRotator PrevRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Falling"))
	bool Falling = false;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Feet Data"))
	FFeetData FeetData;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Feet"))
	bool Feet = false;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Left Foot"))
	double LeftFoot = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Right Foot"))
	double RightFoot = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Left Hand"))
	double LeftHand = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Right Hand"))
	double RightHand = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Pelvis"))
	double Pelvis = 0.0;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Ground Slope"))
	FRotator GroundSlope = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Riding"))
	bool Riding = false;

	UPROPERTY(BlueprintReadOnly, Category = "Default", meta = (DisplayName = "Index"))
	int32 Index = 0;
};
