#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CCharacterInfoComponent.generated.h"

class UCGameInstance;
struct FCCharacterInfo;
struct FLinearColor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCCharacterInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCCharacterInfoComponent();

protected:
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;

public:
	void SetCharacterName(const FName InNewName);
	void SetGroupIndex(int32 InNewGroupIndex);
	void SetBodyColor(const FLinearColor& InNewColor);

	const FName GetCharacterName() const;
	int32 GetGroupIndex() const;

private:
	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
		FName CurName;

	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
		int32 CurGroupIndex;

	UPROPERTY(EditInstanceOnly, Category = "Status", Meta = (AllowPrivateAccess = true))
		FLinearColor CurBodyColor;

private:
	FCCharacterInfo* CurInfo;
};
