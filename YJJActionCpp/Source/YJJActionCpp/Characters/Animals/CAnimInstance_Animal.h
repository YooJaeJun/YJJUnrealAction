#pragma once
#include "CoreMinimal.h"
#include "Characters/CAnimInstance_Character.h"
#include "CAnimInstance_Animal.generated.h"

UCLASS()
class YJJACTIONCPP_API UCAnimInstance_Animal : public UCAnimInstance_Character
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		bool bRotating;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		bool bRiding;
};
