#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EDirection : uint8
{
	Left,
	Right,
	Forward,
	Back,
	Top,
	Bottom,
	Max
};