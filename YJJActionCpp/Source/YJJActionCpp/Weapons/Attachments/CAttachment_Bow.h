#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAttachment.h"
#include "CAttachment_Bow.generated.h"

class ACAttachment_Arrow;

UCLASS()
class YJJACTIONCPP_API ACAttachment_Bow : public ACAttachment
{
	GENERATED_BODY()

private:
	void CreateArrow();

private:
	UPROPERTY(EditAnywhere, Category = "Arrow")
		TArray<ACAttachment_Arrow*> Arrows;

	UPROPERTY(EditAnywhere, Category = "Arrow")
		ACAttachment_Arrow* Arrow;
};
