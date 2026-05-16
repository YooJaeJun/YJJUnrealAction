#pragma once

#include "CoreMinimal.h"
#include "Characters/CAnimInstance_Character.h"
#include "CAnimInstance_Human.generated.h"

// 인간 전용 AnimInstance — 공통 ABP_Character 그래프는 UCAnimInstance_Character 에 두고,
// 아트/블루프린트에서 이 클래스를 상속해 Human 전용 노드를 추가하면 된다.
UCLASS()
class YJJACTIONCPPUE5_API UCAnimInstance_Human : public UCAnimInstance_Character
{
	GENERATED_BODY()
};
