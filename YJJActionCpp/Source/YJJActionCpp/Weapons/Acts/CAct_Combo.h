#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "CAct_Combo.generated.h"

class ACCommonCharacter;

UCLASS()
class YJJACTIONCPP_API UCAct_Combo : public UCAct
{
	GENERATED_BODY()

public:
	FORCEINLINE void EnableCombo() { bEnable = true; }
	FORCEINLINE void DisableCombo() { bEnable = false; }

public:
	virtual void Act() override;
	virtual void Begin_Act() override;
	virtual void End_Act() override;

public:
	virtual void OnAttachmentBeginOverlap(
		ACCommonCharacter* InAttacker,
		AActor* InAttackCauser,
		ACCommonCharacter* InOther) override;

	virtual void OnAttachmentEndCollision() override;

private:
	int32 Index;

	bool bEnable;
	bool bExist;

	TArray<TWeakObjectPtr<ACCommonCharacter>> Hitted;
};
