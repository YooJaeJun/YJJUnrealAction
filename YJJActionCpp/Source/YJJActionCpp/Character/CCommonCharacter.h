#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CStateComponent.h"
#include "Components/CMovementComponent.h"
#include "Components/CCharacterInfoComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "CCommonCharacter.generated.h"

UCLASS(Abstract)
class YJJACTIONCPP_API ACCommonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ACCommonCharacter();

protected:
	virtual void BeginPlay() override;

public:
	// 라이딩 시 PlayerController와 AIController 전환 시 사용. 다른 클래스에서 접근 시 GetController 등이 아닌, 이 함수로 접근
	FORCEINLINE TWeakObjectPtr<AController> GetMyCurController() const { return MyCurController; }

public:
	UPROPERTY(VisibleAnywhere)
		UCStateComponent* StateComponent;

	UPROPERTY(VisibleAnywhere)
		UCMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "Status")
		UCCharacterInfoComponent* CharacterInfoComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Status")
		UCCharacterStatComponent* CharacterStatComponent;

public:
	TWeakObjectPtr<AController> MyCurController;
};
