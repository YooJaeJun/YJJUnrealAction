#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAct.h"
#include "Weapons/ICombatActionHost.h"
#include "CAct_Combo.generated.h"

class ACCommonCharacter;

UCLASS(Blueprintable)
class YJJACTIONCPPUE5_API UCAct_Combo : public UCAct, public ICombatActionHost
{
	GENERATED_BODY()

public:
	FORCEINLINE void EnableCombo() { bEnable = true; }
	FORCEINLINE void DisableCombo() { bEnable = false; }

	virtual void Host_EnableCombo() override;
	virtual void Host_DisableCombo() override;

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
	/** 몽타주·End 노티 없이 Act 에 남은 경우 End_Act 와 동일 경로로 정리(콤보 윈도 bEnable 중에는 호출하지 않음). */
	void Combo_ClearStaleActStateIfNeeded();

	int32 Index;

	bool bEnable;
	bool bExist;

	TArray<TWeakObjectPtr<ACCommonCharacter>> Hitted;
};
