#include "Weapons/Bow/CSkillAim.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Utilities/CLog.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

ACSkillAim::ACSkillAim()
{
	PrimaryActorTick.bCanEverTick = false;

	YJJHelpers::GetClass<UUserWidget>(
		&CrossHairWidgetClass,
		TEXT("WidgetBlueprint'/Game/Widgets/Weapons/WB_CrossHair.WB_CrossHair_C'"));
}

void ACSkillAim::BeginPlay()
{
	Super::BeginPlay();

	if (false == IsValid(Character))
	{
		CLog::Log(FString::Printf(TEXT("[SkillAim] Character 없음(Skill_Weapon 초기화 실패 가능) — %s"), *GetName()));
		return;
	}

	SpringArm = Character->FindComponentByClass<USpringArmComponent>();
	if (false == IsValid(SpringArm))
		CLog::Log(FString::Printf(TEXT("[SkillAim] SpringArmComponent 없음 — %s"), *GetName()));

	Camera = Character->FindComponentByClass<UCameraComponent>();
	if (false == IsValid(Camera))
		CLog::Log(FString::Printf(TEXT("[SkillAim] CameraComponent 없음 — %s"), *GetName()));

	TWeakObjectPtr<AController> curController = Character->GetMyCurController();
	if (false == curController.IsValid())
		curController = Character->GetController();

	if (false == curController.IsValid())
	{
		CLog::Log(FString::Printf(
			TEXT("[SkillAim] CurController/GetController 없음 — CrossHair 생략 — %s"),
			*GetName()));
		return;
	}

	AimController = curController.Get();

	APlayerController* playerController = Cast<APlayerController>(AimController.Get());
	if (false == IsValid(playerController))
	{
		CLog::Log(FString::Printf(TEXT("[SkillAim] PlayerController 아님 — CrossHair 생략 — %s"), *GetName()));
		return;
	}

	if (false == IsValid(CrossHairWidgetClass))
	{
		CLog::Log(FString::Printf(TEXT("[SkillAim] CrossHairWidgetClass 미설정 — %s"), *GetName()));
		return;
	}

	CrossHair = CreateWidget<UUserWidget>(playerController, CrossHairWidgetClass);
	if (IsValid(CrossHair))
	{
		CrossHair->AddToViewport(0);
		CrossHair->SetVisibility(ESlateVisibility::Hidden);
	}
	else
		CLog::Log(FString::Printf(TEXT("[SkillAim] CrossHair 위젯 생성 실패 — %s"), *GetName()));
}

void ACSkillAim::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(CrossHair))
	{
		CrossHair->RemoveFromParent();
		CrossHair = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
