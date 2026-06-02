#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "COwnerActor.generated.h"

class ACCommonCharacter;

// BP Magic / Weapon 이 공유하는 Character 슬롯(스폰 액터 조상).
UCLASS(Abstract)
class YJJACTIONCPPUE5_API ACOwnerActor : public AActor
{
	GENERATED_BODY()

public:
	// Magic/Weapon BP BeginPlay 에서 Owner 를 Cast 해 채우므로 블루프린트에서 쓰기 허용한다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill")
	TObjectPtr<ACCommonCharacter> Character;
};
