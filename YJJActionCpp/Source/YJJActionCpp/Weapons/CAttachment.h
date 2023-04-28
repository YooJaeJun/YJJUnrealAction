#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CAttachment.generated.h"

class ACCommonCharacter;
class USceneComponent;

UCLASS()
class YJJACTIONCPP_API ACAttachment : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		USceneComponent* Root;
	
public:	
	ACAttachment();

protected:
	virtual void BeginPlay() override;

protected:
	TWeakObjectPtr<ACCommonCharacter> Owner;
	//UPROPERTY(BlueprintReadOnly, Category = "Game")
	//	ACCommonCharacter* Owner;
};
