#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CWeaponAsset.generated.h"

class ACAttachment;
class ACCommonCharacter;

UCLASS()
class YJJACTIONCPP_API UCWeaponAsset : public UDataAsset
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
		TSubclassOf<ACAttachment> AttachmentClass;

	UPROPERTY()
		ACAttachment* Attachment;

public:
	FORCEINLINE ACAttachment* GetAttachment() { return Attachment; }

public:
	UCWeaponAsset();

	void BeginPlay(ACCommonCharacter* InOwner);
};
