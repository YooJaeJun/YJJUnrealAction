#pragma once
#include "CoreMinimal.h"
#include "Commons/CGameInstance.h"
#include "Components/ActorComponent.h"
#include "Commons/CEnums.h"
#include "CCharacterInfoComponent.generated.h"

class UCGameInstance;
struct FCCharacterInfo;
struct FLinearColor;
class ACCommonCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class YJJACTIONCPP_API UCCharacterInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCCharacterInfoComponent();

protected:
	virtual void BeginPlay() override;

public:
	bool IsSameGroup(TWeakObjectPtr<ACCommonCharacter> InOther) const;

	void SetCharacterType(const CECharacterType InNewType);
	FORCEINLINE void SetCharacterGroup(const int32 InNewGroup) { CurGroup = InNewGroup; }
	FORCEINLINE void SetCharacterName(const FName& InNewName) { CurName = InNewName; }
	FORCEINLINE void SetBodyColor(const FLinearColor& InNewColor) { CurBodyColor = InNewColor; }

	FORCEINLINE constexpr int32 GetCharacterType() const { return CurInfo.Type; }
	FORCEINLINE constexpr int32 GetCharacterGroup() const { return CurInfo.Group; }
	FORCEINLINE FName GetCharacterName() const { return CurInfo.Name; }

public:
	UPROPERTY(EditAnywhere)
		int32 CurType;

private:
	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
		int32 CurGroup;

	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
		FName CurName;

	UPROPERTY(EditInstanceOnly, Meta = (AllowPrivateAccess = true))
		FLinearColor CurBodyColor;

private:
	FCCharacterInfo CurInfo;
};
