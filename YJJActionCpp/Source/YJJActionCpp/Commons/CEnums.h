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

UENUM(BlueprintType)
enum class EBehaviorType : uint8
{
	Wait,
	Patrol,
	Approach,
	Action,
	Dead,
	DownFlying,
	DownLand,
	Ride,
	Max
};

UENUM(BlueprintType)
enum class ECharacterType : uint8
{
	None,
	Player,
	Companion,
	Enemy_1,
	Enemy_2
};