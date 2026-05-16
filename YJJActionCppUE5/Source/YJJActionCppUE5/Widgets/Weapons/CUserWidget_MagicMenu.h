#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "Commons/CEnums.h"
#include "CUserWidget_MagicMenu.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipMagic, CEMagicType, InNewType);

// BP WB_MagicMenu 용 베이스. BP_Player::SetMenuUI 에서 OnEquipMagic 을 캐릭터로 연결한다.
UCLASS()
class YJJACTIONCPPUE5_API UCUserWidget_MagicMenu : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	// HUD::SetChildren 가 EquipMenu 와 같은 타이밍으로 호출한다.
	void BindChildren();

public:
	UPROPERTY(BlueprintAssignable, Category = "Magic")
	FOnEquipMagic OnEquipMagic;

	UPROPERTY(BlueprintReadWrite, Category = "Magic")
	CEMagicType CurMagicType = CEMagicType::Unarmed;
};
