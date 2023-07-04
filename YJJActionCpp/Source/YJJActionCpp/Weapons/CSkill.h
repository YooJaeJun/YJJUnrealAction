#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CSkill.generated.h"

class ACCommonCharacter;
class ACAttachment;
class UCAct;
class UCStateComponent;
class UCMovementComponent;

UCLASS()
class YJJACTIONCPP_API UCSkill : public UObject
{
	GENERATED_BODY()

public:
	UCSkill();

public:
	virtual void BeginPlay(
		TWeakObjectPtr<ACCommonCharacter> InOwner,
		TWeakObjectPtr<ACAttachment> InAttachment,
		TWeakObjectPtr<UCAct> InAct);

	virtual void Pressed() {}
	virtual void Released() {}

public:
	UFUNCTION(BlueprintNativeEvent)
		void Begin_Skill();
	virtual void Begin_Skill_Implementation() {}

	UFUNCTION(BlueprintNativeEvent)
		void End_Skill();
	virtual void End_Skill_Implementation() {}

	UFUNCTION(BlueprintNativeEvent)
		void Tick(float InDeltaTime);
	virtual void Tick_Implementation(float InDeltaTime) {}

protected:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<ACAttachment> Attachment;
	TWeakObjectPtr<UCAct> Act;

	TWeakObjectPtr<UCStateComponent> StateComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;
};
