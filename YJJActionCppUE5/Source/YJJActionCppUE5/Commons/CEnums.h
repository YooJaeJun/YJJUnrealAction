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
// - EDistanceType     -> CEDistanceType (UDE 정수값·항목 순서 에디터 에셋과 동기화 필수)
// - EDragonBossBehaviorType -> CEDragonBossBehaviorType
// - EDragonBossSkillType    -> CEDragonBossSkillType (드래곤 보스 패턴 전용 스킬 6종 + Max)
// - ESkillType        -> 레거시 UDE 이름이 블프마다 상이함. 드래곤 무기 분기에서는 EDragonBossSkillType 과 순서 동일해야 함(`CEDragonBossSkillType` 사용 권장)
// - ETeleportProcessType    -> CETeleportProcessType

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

/**
 * 레거시 `/Game/.../EDragonBossSkillType`(스크린샷 순: Roar, Fire, Meteor, FireFly, Thunder, Fly).
 * 무기 `Skill` 바이트·BT 블랙보드 저장값과 순서 고정 동형(Roar=0 … Fly=5).
 * 레거시 `ESkillType` UDE 의 드래곤 무기 채널이 위와 동일하면 본 타입으로 통일하면 된다.
 */
UENUM(BlueprintType)
enum class CEDragonBossSkillType : uint8
{
	Roar UMETA(DisplayName = "Roar"),
	Fire UMETA(DisplayName = "Fire"),
	Meteor UMETA(DisplayName = "Meteor"),
	FireFly UMETA(DisplayName = "FireFly"),
	Thunder UMETA(DisplayName = "Thunder"),
	Fly UMETA(DisplayName = "Fly"),
	Max UMETA(Hidden),
};

/**
 * 레거시 `/Game/.../EDistanceType`.
 * 블프 UDE 에 항목이 더 있거나 정수표기가 바뀌면 `UCBTService_DragonBoss` 의 DistanceTypeBB_* 과 함께 수정한다.
 * 아래 명시값은 해당 서비스 노드 디폴트와 일치하게 둔다.
 */
UENUM(BlueprintType)
enum class CEDistanceType : uint8
{
	/** UHT·블프 기본값(0) 요구 및 미초기화 슬롯용 — 레거시 UDE 본 편값은 아님(`CBTTask` 등은 Approach=11 과 동형 유지). */
	None = 0 UMETA(Hidden),
	Approach = 11 UMETA(DisplayName = "Approach"),
	Skill = 13 UMETA(DisplayName = "Skill"),
	Action = 14 UMETA(DisplayName = "Action"),
	Avoid = 18 UMETA(DisplayName = "Avoid"),

	Max UMETA(Hidden),
};

/**
 * 레거시 `/Game/.../EDragonBossBehaviorType`(스크린샷 순: Cinematic, Wait, Groggy, Dead, Action, Riding, Skill, Approach).
 * `Skill`/`Approach` 는 UDE 에서 BT 가 쓰는 바이트(12, 14)와 맞도록 명시하였다(Action=4 는 UCBTService_DragonBoss 디폴트와 동형).
 */
UENUM(BlueprintType)
enum class CEDragonBossBehaviorType : uint8
{
	Cinematic = 0 UMETA(DisplayName = "Cinematic"),
	Wait = 1 UMETA(DisplayName = "Wait"),
	Groggy = 2 UMETA(DisplayName = "Groggy"),
	Dead = 3 UMETA(DisplayName = "Dead"),
	Action = 4 UMETA(DisplayName = "Action"),
	Riding = 5 UMETA(DisplayName = "Riding"),
	Skill = 12 UMETA(DisplayName = "Skill"),
	Approach = 14 UMETA(DisplayName = "Approach"),

	Max UMETA(Hidden),
};

/** 레거시 `/Game/.../ETeleportProcessType`(스크린샷 순, None부터 Appeared 까지 0..6). */
UENUM(BlueprintType)
enum class CETeleportProcessType : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Execute UMETA(DisplayName = "Execute"),
	Disappearing UMETA(DisplayName = "Disappearing"),
	Disappeared UMETA(DisplayName = "Disappeared"),
	Warping UMETA(DisplayName = "Warping"),
	Appearing UMETA(DisplayName = "Appearing"),
	Appeared UMETA(DisplayName = "Appeared"),

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

	// 레거시 CABP BlendList(EWeaponType)·마법 무기 블렌드 핀 이름 — AnimBP 재컴파일 시 값 추가로 인덱스가 밀릴 수 있다.
	Guard UMETA(DisplayName = "Guard"),
	Warp UMETA(DisplayName = "Warp"),
	Around UMETA(DisplayName = "Around"),
	Fireball UMETA(DisplayName = "FireBall"),
	Bomb UMETA(DisplayName = "Bomb"),
	Yondu UMETA(DisplayName = "Yondu"),
	Sword_Hook UMETA(DisplayName = "Sword Hook"),

	Max UMETA(Hidden),
};

// BB_Enemy 블랙보드 Behavior 키(Switch / SetBlackboard 같은 연동 시 바이트 순서 이슈 회피)용.
// /Game/…/EBehaviorType UDE 열거 순서(Wait→…→Riding)와 반드시 동일해야 한다.
UENUM(BlueprintType)
enum class CEBehaviorType : uint8
{
	Wait,
	Approach,
	Action,
	Patrol,
	CombatHitted UMETA(DisplayName = "Hitted"),
	Dead,
	Avoid,
	Flying,
	Falling,
	HitAir,
	DownFlying,
	DownLand,
	Rise,
	Riding,

	Max UMETA(Hidden),
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