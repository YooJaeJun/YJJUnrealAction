#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "CFeetComponent.generated.h"

class ACCommonCharacter;

USTRUCT(BlueprintType)
struct FFeetData
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Feet")
        FVector LeftDistance; //X

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Feet")
        FVector RightDistance; //X

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Feet")
        FVector PelvisDistance; //Z

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Feet")
        FRotator LeftRotation;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Feet")
        FRotator RightRotation;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCFeetComponent : public UActorComponent
{
	GENERATED_BODY()

private:
    UPROPERTY(EditAnywhere, Category = "Trace")
        TEnumAsByte<EDrawDebugTrace::Type> DrawDebug;

    UPROPERTY(EditAnywhere, Category = "Trace")
        float InterpSpeed = 50;

    UPROPERTY(EditAnywhere, Category = "Trace")
        float TraceDistance = 50;

    UPROPERTY(EditAnywhere, Category = "Trace")
        float OffsetDistance = 5;

    UPROPERTY(EditAnywhere, Category = "Socket")
        FName LeftSocket = "Foot_L";

    UPROPERTY(EditAnywhere, Category = "Socket")
        FName RightSocket = "Foot_R";

public:
    FORCEINLINE FFeetData GetData() const { return Data; }

public:	
	UCFeetComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void Trace(const FName InName, float& OutDistance, FRotator& OutRotation) const;

private:
    TWeakObjectPtr<ACCommonCharacter> Owner;
    FFeetData Data;
};
