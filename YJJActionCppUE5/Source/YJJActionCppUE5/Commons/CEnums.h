#pragma once
#include "CoreMinimal.h"

// 블루프린트 UserDefinedEnum 에셋 이름 ↔ C++ 타입(프로젝트 규칙: CE 접두사)
// - EDirection        -> CEDirection (BP 는 보통 Forward/Left/Right/Back 4방; C++ 에 Top/Bottom 추가)
// - ESpeedType        -> CESpeedType
// - EStateType        -> CEStateType (Avoid·Legacy_MaxSlot 등 C++ 전용 값 포함)
// - ERidingState      -> CERidingState
// - EAttackType       -> CEAttackType
// - ECrowdControl     -> CECrowdControl
// - EMagicType        -> CEMagicType
// - EWeaponType       -> CEWeaponType

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
	// UHT 는 "이름 UMETA(...) = 값" 순서가 아니라 "=" 앞에는 식별자만 오도록 한다.
	Idle = 0 UMETA(DisplayName = "Idle"),
	Fall = 1 UMETA(DisplayName = "Falling"),
	Avoid = 2 UMETA(DisplayName = "Avoid"),
	Equip = 3 UMETA(DisplayName = "Equip"),
	Act = 4 UMETA(DisplayName = "Action"),
	Rise = 5 UMETA(DisplayName = "Rise"),
	Land = 6 UMETA(DisplayName = "DownLand"),
	Dead = 7 UMETA(DisplayName = "Dead"),
	Riding = 8 UMETA(DisplayName = "Riding"),

	// 구버전 CEStateType::Max 가 9 를 쓰던 자리 유지 — 세이브/PrevType 과 충돌 방지용(미사용 플레이셀더).
	Legacy_MaxSlot = 9 UMETA(Hidden),

	Cinematic = 10 UMETA(DisplayName = "Cinematic"),
	Dash = 11 UMETA(DisplayName = "Dash"),
	Parkour = 12 UMETA(DisplayName = "Parkour"),
	Flying = 13 UMETA(DisplayName = "Flying"),
	HitAir = 14 UMETA(DisplayName = "HitAir"),
	DownFlying = 15 UMETA(DisplayName = "DownFlying"),
	Groggy = 16 UMETA(DisplayName = "Groggy"),
	CombatHitted = 17 UMETA(DisplayName = "Hitted"),

	Max UMETA(Hidden),
};

UENUM(BlueprintType)
enum class CERidingState : uint8
{
	None,
	MovingToMountPoint,
	Mounting,
	MountingEnd,
	Riding,
	Unmounting,
	RidingEnd
};

UENUM(BlueprintType)
enum class CEAttackType : uint8
{
	Common,
	Air,
	Flying,
	Down,
	DashAttack,
	FallDown,
	Dash,
	AirDash,
	Skill,
	Riding,
	Max UMETA(Hidden),
};

/** BP ECrowdControl 순서를 그대로 둔다. None 은 상태 없음 표기용이다. */
UENUM(BlueprintType)
enum class CECrowdControl : uint8
{
	None,
	Air,
	PutDown,
	Down,
	Max UMETA(Hidden),
};

/** BP EMagicType — 물리 무기 CEWeaponType 과 분리(마법 전용 장비 DA 키). */
UENUM(BlueprintType)
enum class CEMagicType : uint8
{
	Unarmed,
	Warp,
	Around,
	FireBall,
	Bomb,
	Yondu,
	Max UMETA(Hidden),
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

// 피격 반응(CEHitType)은 애니·스테이트용이므로, 공격 분류 CEAttackType 은 여기서 별도 매핑한다.
FORCEINLINE CEHitType CEHitReactionFromAttackType(CEAttackType InAttackType)
{
	switch (InAttackType)
	{
	case CEAttackType::Common:
	case CEAttackType::Skill:
	case CEAttackType::Riding:
		return CEHitType::Common;
	case CEAttackType::Air:
	case CEAttackType::AirDash:
		return CEHitType::Air;
	case CEAttackType::Flying:
		return CEHitType::Fly;
	case CEAttackType::Down:
		return CEHitType::Down;
	case CEAttackType::DashAttack:
	case CEAttackType::Dash:
		return CEHitType::Knockback;
	case CEAttackType::FallDown:
		return CEHitType::FlyingPutDown;
	default:
		return CEHitType::Common;
	}
}

UENUM(BlueprintType)
enum class CESpeedType : uint8
{
	Walk,
	Run,
	Sprint,
	Max
};

UENUM(BlueprintType)
enum class CEWeaponType : uint8
{
	Unarmed,
	Fist,
	Sword,
	Hammer,
	Bow,
	Dual,
	Shield,

	Max UMETA(Hidden),
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

/** BP EParkourType — 행 이름·데이터 테이블 타입열과 순서 재정렬 시 에셋을 다시 매핑해야 한다. */
UENUM(BlueprintType)
enum class CEParkourType : uint8
{
	None UMETA(DisplayName = "None"),

	Vault UMETA(DisplayName = "Vault"),
	Climb UMETA(DisplayName = "Climb"),
	Mantle UMETA(DisplayName = "Mantle"),
	Slide UMETA(DisplayName = "Slide"),

	Ledge UMETA(DisplayName = "Ledge"),
	Wallrun UMETA(DisplayName = "Wallrun"),
	Hanging UMETA(DisplayName = "Hanging"),

	Max UMETA(Hidden),
};

/** BP EParkourArrowType — 순서와 Ceil/Center/… 이름이 플레이어 ArrowGroup 자식 이름과 매칭된다. */
UENUM(BlueprintType)
enum class CEParkourArrowType : uint8
{
	Ceil UMETA(DisplayName = "Ceil"),
	Center UMETA(DisplayName = "Center"),
	Floor UMETA(DisplayName = "Floor"),
	Land UMETA(DisplayName = "Land"),
	Left UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right"),

	Max UMETA(Hidden),
};