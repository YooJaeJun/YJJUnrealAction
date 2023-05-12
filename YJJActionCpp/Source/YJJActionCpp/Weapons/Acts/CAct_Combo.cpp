#include "Weapons/Acts/CAct_Combo.h"
#include "Global.h"
#include "GameFramework/Character.h"
#include "Components/CStateComponent.h"
#include "Character/CCommonCharacter.h"
#include "UMG/Public/Blueprint/WidgetLayoutLibrary.h"

void UCAct_Combo::Act()
{
	CheckTrue(ActDatas.Num() < 1);

	if (bEnable)
	{
		bEnable = false;
		bExist = true;

		return;
	}

	CheckFalse(StateComp->IsIdleMode());

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

void UCAct_Combo::OnAttachmentBeginOverlap(ACCommonCharacter* InAttacker, AActor* InAttackCauser,
                                           ACCommonCharacter* InOther)
{
	Super::OnAttachmentBeginOverlap(InAttacker, InAttackCauser, InOther);
	CheckNull(InOther);

	for (ACCommonCharacter* hitted : Hitted)
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

	for (ACCommonCharacter* hitted : Hitted)
	{
		FVector direction = hitted->GetActorLocation() - Owner->GetActorLocation();
		direction = direction.GetSafeNormal2D();

		FVector forward = FQuat(Owner->GetActorRotation()).GetForwardVector();

		float dot = FVector::DotProduct(direction, forward);
		if (dot >= 0.7f && angle <= dot)
		{
			angle = dot;
			candidate = hitted;
		}
	}

	if (!!candidate.Get())
	{
		FRotator rotator = UKismetMathLibrary::FindLookAtRotation(Owner->GetActorLocation(),
			candidate->GetActorLocation());

		Owner->TogglebTickLerpForTarget();
		Owner->SetTickLerp(FRotator(0, rotator.Yaw, 0));
		Owner->SetMousePos(UWidgetLayoutLibrary::GetMousePositionOnPlatform());
	}

	Hitted.Empty();
}
