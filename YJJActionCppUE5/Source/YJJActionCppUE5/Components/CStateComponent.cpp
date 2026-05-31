#include "Components/CStateComponent.h"

#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "UObject/UnrealType.h"

UCStateComponent::UCStateComponent()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UCStateComponent::CanAttack() const
{
	// 장착 몽타주(Equip) 구간에도 좌클릭 콤보 진입 허용 — End_Equip 전 Idle 만 요구하면 입력이 막힌다.
	return IsIdleMode() || IsEquipMode();
}

void UCStateComponent::SetIdleMode()
{
	InternalSetStateType(CEStateType::Idle);
}

void UCStateComponent::SetFallMode()
{
	InternalSetStateType(CEStateType::Fall);
}

void UCStateComponent::SetAvoidMode()
{
	InternalSetStateType(CEStateType::Avoid);
}

void UCStateComponent::SetEquipMode()
{
	InternalSetStateType(CEStateType::Equip);
}

void UCStateComponent::SetActMode()
{
	InternalSetStateType(CEStateType::Act);
}

void UCStateComponent::SetDeadMode()
{
	InternalSetStateType(CEStateType::Dead);
}

void UCStateComponent::SetRiseMode()
{
	InternalSetStateType(CEStateType::Rise);
}

void UCStateComponent::SetRidingMode()
{
	InternalSetStateType(CEStateType::Riding);
}

void UCStateComponent::ChangeType(CEStateType InNewType)
{
	InternalSetStateType(InNewType);
}

void UCStateComponent::SetMode(CEStateType InStateType)
{
	InternalSetStateType(InStateType);
}

void UCStateComponent::SaveType()
{
	// 레거시 BP: PrevType 에 현재 Type 스냅샷만 남김. CurType 과 동기화된다.
	PrevType = CurType;
}

void UCStateComponent::LoadType()
{
	// 레거시 BP: 로컬 Temp 로 Type 과 Prev 를 스왑한 뒤 델리게이트 재발행.
	const CEStateType TempOldCurrent = CurType;
	CurType = PrevType;
	PrevType = TempOldCurrent;
	Type = CurType;

	if (false == OnStateTypeChanged.IsBound())
		return;

	const AActor* Ow = GetOwner();
	if (Ow != nullptr)
	{
		CLog::Print(Ow->GetName() + FString(TEXT(" [LoadType] "))
			+ YJJHelpers::ConvertEnumToString(PrevType) + FString(TEXT(" -> "))
			+ YJJHelpers::ConvertEnumToString(CurType),
			-1,
			5,
			FColor::Emerald);
	}
	else
	{
		CLog::Log(TEXT("[UCStateComponent::LoadType] Owner 없음 — LoadType 브로드캐스트만 수행한다."));
	}

	OnStateTypeChanged.Broadcast(TempOldCurrent, CurType);
}

FString UCStateComponent::GetStringState() const
{
	const UEnum* EnumPtr = StaticEnum<CEStateType>();
	if (nullptr == EnumPtr)
	{
		CLog::Log(TEXT("[UCStateComponent::GetStringState] StaticEnum<CEStateType> 가 null 입니다."));
		return FString();
	}
	return EnumPtr->GetNameStringByValue(static_cast<int64>(CurType));
}

bool UCStateComponent::IsMoveable() const
{
	switch (CurType)
	{
	case CEStateType::Idle:
	case CEStateType::Equip:
	case CEStateType::CombatHitted:
		return true;
	default:
		return false;
	}
}

bool UCStateComponent::IsRealRiding() const
{
	if (false == Owner.IsValid())
	{
		return false;
	}

	if (IsValid(Owner->CurInteractingActor))
	{
		return true;
	}

	return Owner->GetbRiding();
}

bool UCStateComponent::IsRidingRecoverContext() const
{
	return IsRidingMode() || IsRealRiding();
}

bool UCStateComponent::IsRiding() const
{
	// 레거시 BP 의 Is Riding: StateComponent 변수 Type 과 Riding 만 비교.
	return IsRidingMode();
}

void UCStateComponent::InternalSetStateType(CEStateType InType)
{
	if (CurType == InType)
	{
		// 레거시 BP 에서 변경 없음이면 Prev/델리게이트 생략(불필요한 상태 전복 방지).
		return;
	}

	PrevType = CurType;
	CurType = InType;
	Type = CurType;

	if (false == OnStateTypeChanged.IsBound())
	{
		return;
	}

	const AActor* Ow = GetOwner();
	if (Ow != nullptr)
	{
		CLog::Print(Ow->GetName() + FString(TEXT(" : "))
			+ YJJHelpers::ConvertEnumToString(CurType),
			-1,
			5,
			FColor::Emerald);
	}
	else
	{
		CLog::Log(TEXT("[UCStateComponent::InternalSetStateType] Owner 없음 — 상태 델리게이트만 브로드캐스트한다."));
	}

	OnStateTypeChanged.Broadcast(PrevType, CurType);
}

void UCStateComponent::SetHitMode(const CEHitType InHitType)
{
	ChangeHitType(InHitType);
}

void UCStateComponent::SetHitNoneMode()
{
	ChangeHitType(CEHitType::None);
}

void UCStateComponent::SetHitCommonMode()
{
	ChangeHitType(CEHitType::Common);
}

void UCStateComponent::SetHitDownMode()
{
	ChangeHitType(CEHitType::Down);
}

void UCStateComponent::SetHitFlyMode()
{
	ChangeHitType(CEHitType::Fly);
}

void UCStateComponent::SetHitKnockbackMode()
{
	ChangeHitType(CEHitType::Knockback);
}

void UCStateComponent::SetHitReactionAir()
{
	ChangeHitType(CEHitType::Air);
}

void UCStateComponent::SetHitFlyingPutDownMode()
{
	ChangeHitType(CEHitType::FlyingPutDown);
}

void UCStateComponent::ChangeHitType(const CEHitType InType)
{
	PrevHitType = CurHitType;
	CurHitType = InType;

	if (OnHitStateTypeChanged.IsBound())
	{
		const AActor* Ow = GetOwner();
		if (Ow != nullptr)
		{
			CLog::Print(Ow->GetName() + FString(TEXT(" : "))
				+ YJJHelpers::ConvertEnumToString(CurHitType),
				-1,
				5,
				FColor::Orange);
		}
		else
		{
			CLog::Log(TEXT("[UCStateComponent::ChangeHitType] Owner 없음 — 피격 델리게이트만 브로드캐스트한다."));
		}

		OnHitStateTypeChanged.Broadcast(PrevHitType, CurHitType);
	}
}
