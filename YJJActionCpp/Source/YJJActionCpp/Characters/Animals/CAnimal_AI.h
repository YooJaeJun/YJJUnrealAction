#pragma once

#include "CoreMinimal.h"
#include "Characters/Animals/CAnimal.h"
#include "CAnimal_AI.generated.h"

UCLASS(Abstract)
class YJJACTIONCPP_API ACAnimal_AI :
	public ACAnimal
{
	GENERATED_BODY()

public:
	ACAnimal_AI();
};