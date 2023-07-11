#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class CEDirection : uint8
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
enum class CECharacterType : uint8
{
	None,
	Player,
	Companion,
	Enemy_1,
	Enemy_2
};

UENUM(BlueprintType)
enum class CEStateType : uint8
{
	Idle,
	Fall,
	Avoid,
	Equip,
	Act,
	Rise,
	Land,
	Dead,
	Max	
};

UENUM(BlueprintType)
enum class CEHitType : uint8
{
	None,
	Common,
	Knockback,
	Down,
	Air,
	Fly,
	FlyingPutDown,
	Max
};

UENUM(BlueprintType)
enum class CEWeaponType : uint8
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
enum class CEBehaviorType : uint8
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