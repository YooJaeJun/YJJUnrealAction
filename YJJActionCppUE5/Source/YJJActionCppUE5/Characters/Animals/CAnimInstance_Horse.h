#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/CAnimInstance_Animal.h"
#include "CAnimInstance_Horse.generated.h"

// 레거시 ABP_Horse 가 부모로 잡기 위한 빈 서브클래스 — 로직은 전부 UCAnimInstance_Animal.
UCLASS()
class YJJACTIONCPPUE5_API UCAnimInstance_Horse : public UCAnimInstance_Animal
{
	GENERATED_BODY()
};
