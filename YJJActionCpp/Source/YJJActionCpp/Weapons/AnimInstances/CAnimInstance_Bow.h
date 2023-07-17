#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CAnimInstance_Bow.generated.h"

UCLASS()
class YJJACTIONCPP_API UCAnimInstance_Bow : public UAnimInstance
{
	GENERATED_BODY()

public:
	FORCEINLINE float* GetBend() { return &Bend; }

protected:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Animation")
		float Bend;
};