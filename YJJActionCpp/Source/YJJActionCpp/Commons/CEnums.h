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
enum class ECharacterType : uint8
{
	None,
	Player,
	Companion,
	Enemy_1,
	Enemy_2
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Unarmed,
	Fist,
	Sword,
	Hammer,
	Dual,
	Guard,
	Bow,
	Warp,
	Around,
	Fireball,
	Bomb,
	Yondu,
	Animal,
	Sword_Hook,
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

UENUM()
enum class EHitType
{
	None,
	Knockback,
	Air,
	Fly,
	FlyingPutDown,
	Down,
	Max
};