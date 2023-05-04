#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Component/CStateComponent.h"	// Dynamic Multicast Delegate
#include "CCommonCharacter.generated.h"

class UCMovementComponent;

UCLASS()
class YJJACTIONCPP_API ACCommonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
		UCMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
		UCStateComponent* StateComponent;

	UPROPERTY(EditAnywhere)
		FString Name;

public:
	// 라이딩 시 PlayerController와 AIController 전환 시 사용. 다른 클래스에서 접근 시 GetController 등이 아닌, 이 함수로 접근
	FORCEINLINE TWeakObjectPtr<AController> GetMyCurController() const { return MyCurController; }
	FORCEINLINE const FString& GetName() const { return Name; }

public:
	ACCommonCharacter();

protected:
	virtual void BeginPlay() override;

	TWeakObjectPtr<AController> MyCurController;
};
