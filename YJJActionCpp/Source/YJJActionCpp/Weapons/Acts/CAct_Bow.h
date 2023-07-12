#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "CAct_Bow.generated.h"

class ACCommonCharacter;

UCLASS(Blueprintable)
class YJJACTIONCPP_API UCAct_Bow : public UCAct
{
	GENERATED_BODY()

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
	TArray<TWeakObjectPtr<ACCommonCharacter>> Hitted;
};
