#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CSkill.generated.h"

class ACCommonCharacter;
class ACAttachment;
class UCAct;
class UCStateComponent;
class UCMovementComponent;

UCLASS(Abstract)
class YJJACTIONCPP_API UCSkill : public UObject
{
	GENERATED_BODY()

public:
	UCSkill();

public:
	virtual void BeginPlay(
		TWeakObjectPtr<ACCommonCharacter> InOwner,
		ACAttachment* InAttachment,
		UCAct* InAct);

	UFUNCTION(BlueprintNativeEvent)
		void Tick(float InDeltaTime);
	virtual void Tick_Implementation(float InDeltaTime) {}

	virtual void Pressed() {}
	virtual void Released() {}

	UFUNCTION(BlueprintNativeEvent)
		void Begin_Skill();
	virtual void Begin_Skill_Implementation();

	UFUNCTION(BlueprintNativeEvent)
		void End_Skill();
	virtual void End_Skill_Implementation();

	FORCEINLINE bool GetInAction() const { return bInAction; }

protected:
	UPROPERTY(EditAnywhere, Category = "Act")
		UCAct* Act;

	UPROPERTY(EditAnywhere, Category = "Act")
		ACAttachment* Attachment;

protected:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	TWeakObjectPtr<UCStateComponent> StateComp;
	TWeakObjectPtr<UCMovementComponent> MovementComp;

	bool bInAction = false;
};