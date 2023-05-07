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
	
public:	
	ACAttachment();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintImplementableEvent)
		void OnBeginEquip();

	UFUNCTION(BlueprintImplementableEvent)
		void OnUnequip();

protected:
	UFUNCTION(BlueprintCallable, Category = "Attach")
		void AttachTo(FName InSocketName);

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		USceneComponent* Root;

protected:
	TWeakObjectPtr<ACCommonCharacter> Owner;
};
