#pragma once

#include "CoreMinimal.h"
#include "Weapons/CAttachment.h"
#include "CAttachment_Animal.generated.h"

// 레거시 `/Game/Weapons/Animal/CBP_Attachment_Animal` — Root 직계 자식 이름으로 캐릭터 메시 소켓에 Shape 부착.
UCLASS()
class YJJACTIONCPPUE5_API ACAttachment_Animal : public ACAttachment
{
	GENERATED_BODY()

public:
	ACAttachment_Animal();

protected:
	virtual void BeginPlay() override;

private:
	void Animal_AttachDirectChildrenToCollisionsReverse();
};
