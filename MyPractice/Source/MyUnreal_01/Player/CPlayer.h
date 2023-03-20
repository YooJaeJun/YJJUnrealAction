#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CPlayer.generated.h"

UCLASS(BlueprintType, Blueprintable,
	meta=(ShortTooltip="Base Class for any Action type"))
class MYUNREAL_01_API UAction : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties) FString Text;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties) FKey ShortcutKey;
};

UENUM()
enum Status
{
	Stopped UMETA(DisplayName = "Stopped"),
	Moving UMETA(DisplayName = "Moving"),
	Attacking UMETA(DisplayName = "Attacking"),
};

USTRUCT(Blueprintable)
struct FColoredTexture
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
		UTexture* Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
		FLinearColor Color;
};

UCLASS()
class MYUNREAL_01_API ACPlayer : public ACharacter
{
	GENERATED_BODY()
	
public:	
	ACPlayer();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Unit)
		TSubclassOf<UObject> UClassOfPlayer;

	UPROPERTY(EditAnywhere, meta = (MetaClass = "GameMode"), Category = Unit)
		FStringClassReference UClassGameMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
		FColoredTexture Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Status)
		TEnumAsByte<Status> Status;
};
