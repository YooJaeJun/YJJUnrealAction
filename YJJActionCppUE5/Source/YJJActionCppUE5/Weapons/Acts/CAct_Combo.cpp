#include "Weapons/Acts/CAct_Combo.h"
#include "Global.h"
#include "Components/CStateComponent.h"
#include "Components/CCharacterStatComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Components/CCharacterInfoComponent.h"
#include "GameFramework/Character.h"
#include "UMG/Public/Blueprint/WidgetLayoutLibrary.h"

void UCAct_Combo::Combo_ClearStaleActStateIfNeeded()
{
	if (false == StateComp.IsValid())
	{
		return;
	}

	if (false == StateComp->IsActMode())
	{
		return;
	}

	ACharacter* const character = Cast<ACharacter>(Owner.Get());
	if (false == IsValid(character))
	{
		return;
	}

	// 몽타주 재생 중이면 정상 Act 구간 — End_Act/End_Action 대기.
	if (IsValid(character->GetCurrentMontage()))
	{
		return;
	}

	End_Act();
}

void UCAct_Combo::Host_EnableCombo()
{
	EnableCombo();
}

void UCAct_Combo::Host_DisableCombo()
{
	DisableCombo();
}

void UCAct_Combo::Act()
{
	if (ActDatas.Num() < 1)
	{
		const AActor* const ownerActor = Owner.Get();
		CLog::Log(FString::Printf(
			TEXT("[입력][Action] ActDatas 비어 있음 — DT_DoAction RowStruct(FDoActionData) 확인 Owner=%s"),
			IsValid(ownerActor) ? *ownerActor->GetName() : TEXT("(null)")));
		return;
	}

	if (bEnable)
	{
		bEnable = false;
		bExist = true;

		return;
	}

	Combo_ClearStaleActStateIfNeeded();

	if (false == StateComp->CanAttack())
	{
		const AActor* const ownerActor = Owner.Get();
		CLog::Log(FString::Printf(
			TEXT("[입력][Action] CanAttack=false — CurState=%s Owner=%s"),
			*YJJHelpers::ConvertEnumToString(StateComp->GetCurMode()),
			IsValid(ownerActor) ? *ownerActor->GetName() : TEXT("(null)")));
		return;
	}

	const UCCharacterStatComponent* statComp =
		Owner.IsValid()
			? Cast<UCCharacterStatComponent>(Owner->GetComponentByClass(UCCharacterStatComponent::StaticClass()))
			: nullptr;

	if (false == IsValid(statComp))
	{
		const AActor* const ownerActor = Owner.Get();
		CLog::Log(FString::Printf(
			TEXT("[입력][Action] StatComp 없음 — Owner=%s"),
			IsValid(ownerActor) ? *ownerActor->GetName() : TEXT("(null)")));
		return;
	}

	if (false == ActDatas.IsValidIndex(Index))
	{
		return;
	}

	const FDoActionData& rowScratch = ActDatas[Index];
	if (statComp->GetCurStamina() < rowScratch.Stamina)
	{
		const AActor* const ownerActor = Owner.Get();
		CLog::Log(FString::Printf(
			TEXT("[입력][Action] 스태미나 부족 — 필요=%.1f 현재=%.1f Owner=%s"),
			rowScratch.Stamina,
			statComp->GetCurStamina(),
			IsValid(ownerActor) ? *ownerActor->GetName() : TEXT("(null)")));
		return;
	}

	Super::Act();

	ActDatas[Index].Act(Owner.Get());
}

void UCAct_Combo::Begin_Act()
{
	Super::Begin_Act();
	CheckFalse(bExist);

	bExist = false;
	ActDatas[++Index].Act(Owner.Get());
}

void UCAct_Combo::End_Act()
{
	Super::End_Act();

	Index = 0;
}

void UCAct_Combo::OnAttachmentBeginOverlap(
	ACCommonCharacter* InAttacker,
	AActor* InAttackCauser,
	ACCommonCharacter* InOther)
{
	CheckNull(InOther);

	const TWeakObjectPtr<UCCharacterInfoComponent> characterInfoComp = 
		YJJHelpers::GetComponent<UCCharacterInfoComponent>(InAttacker);

	CheckNull(characterInfoComp);
	CheckTrue(characterInfoComp->IsSameGroup(InOther));

	Super::OnAttachmentBeginOverlap(InAttacker, InAttackCauser, InOther);

	for (const TWeakObjectPtr<ACCommonCharacter> hitted : Hitted)
		CheckTrue(hitted == InOther);

	Hitted.AddUnique(InOther);

	CheckTrue(HitDatas.Num() - 1 < Index);

	HitDatas[Index].SendDamage(InAttacker, InAttackCauser, InOther);
}

void UCAct_Combo::OnAttachmentEndCollision()
{
	Super::OnAttachmentEndCollision();

	float angle = -1.0f;
	TWeakObjectPtr<ACCommonCharacter> candidate = nullptr;

	for (const TWeakObjectPtr<ACCommonCharacter> hitted : Hitted)
	{
		FVector direction = hitted->GetActorLocation() - Owner->GetActorLocation();
		direction = direction.GetSafeNormal2D();

		const FVector forward = FQuat(Owner->GetActorRotation()).GetForwardVector();

		const float dot = FVector::DotProduct(direction, forward);
		if (dot >= 0.7f && angle <= dot)
		{
			angle = dot;
			candidate = hitted;
		}
	}

	if (candidate.IsValid())
	{
		const FRotator rotator = UKismetMathLibrary::FindLookAtRotation(
			Owner->GetActorLocation(),
			candidate->GetActorLocation());

		Owner->TogglebTickLerpForTarget();
		Owner->SetTickLerp(FRotator(0, rotator.Yaw, 0));
		Owner->SetMousePos(UWidgetLayoutLibrary::GetMousePositionOnPlatform());
	}

	Hitted.Empty();
}
