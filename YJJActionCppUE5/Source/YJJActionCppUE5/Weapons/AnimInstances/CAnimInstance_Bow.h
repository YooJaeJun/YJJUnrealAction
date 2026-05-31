#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CAnimInstance_Bow.generated.h"

UCLASS()
class YJJACTIONCPPUE5_API UCAnimInstance_Bow : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;

	// ABP Bend 프로퍼티와 동일 메모리를 가리킨다(에이밍·Act가 *Bend로 갱신).
	TSharedPtr<float> GetBend();

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
	float Bend = 0.0f;

private:
	TSharedPtr<float> BendShared;
};
