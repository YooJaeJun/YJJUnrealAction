#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CFlyComponent.generated.h"

class ACCommonCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCFlyComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCFlyComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InputAxis_MoveForward(const float InAxis);
	void InputAxis_MoveRight(const float InAxis);
	void InputAxis_HorizontalLook(const float InAxis);
	void InputAxis_VerticalLook(const float InAxis);
	void InputAxis_FlyUp(const float InAxis);
	void InputAction_Jump();

	bool IsFlying() const;

private:
	UPROPERTY(EditAnywhere, Category = "Camera")
		float HorizontalLook = 45;

	UPROPERTY(EditAnywhere, Category = "Camera")
		float VerticalLook = 45;

	UPROPERTY(EditAnywhere, Category = "Move")
		float MoveingFactor = 1000;

	UPROPERTY(EditAnywhere, Category = "Move")
		float InterpSpeed = 2;

private:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};