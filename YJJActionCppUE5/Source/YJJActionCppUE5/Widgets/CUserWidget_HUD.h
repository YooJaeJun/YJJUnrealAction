#pragma once

#include "CoreMinimal.h"
#include "Widgets/CUserWidget_Custom.h"
#include "CUserWidget_HUD.generated.h"

class UCUserWidget_PlayerInfo;
class UCUserWidget_EquipMenu;
class UCUserWidget_MagicMenu;
class UCUserWidget_Interaction;

UCLASS()
class YJJACTIONCPPUE5_API UCUserWidget_HUD : public UCUserWidget_Custom
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetChildren();

	UFUNCTION(BlueprintPure, Category = "HUD")
	UCUserWidget_PlayerInfo* GetPlayerInfoWidget() const { return PlayerInfo; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	UCUserWidget_EquipMenu* GetEquipMenuWidget() const { return EquipMenu; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	UCUserWidget_MagicMenu* GetMagicMenuWidget() const { return MagicMenu; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	UCUserWidget_Interaction* GetInteractionWidget() const { return Interaction; }

	// 레거시 WB_HUDUI::WB_Message — 이름으로 바인드. C 접두 블루프린트 레이아웃용 보조 이름도 시도한다.
	UUserWidget* ResolveWB_MessageWidget();

	UFUNCTION(BlueprintPure, Category = "HUD", meta = (DisplayName = "WB Message Widget"))
	UUserWidget* GetWB_MessageWidgetReadOnly() const { return HudMessageSlotWidget; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player")
	TObjectPtr<UCUserWidget_PlayerInfo> PlayerInfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
	TObjectPtr<UCUserWidget_EquipMenu> EquipMenu;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapons")
	TObjectPtr<UCUserWidget_MagicMenu> MagicMenu;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UCUserWidget_Interaction> Interaction;

	// UMG 에서 위젯 트리 이름이 "WB_Message" 인 노드와 동명인 UPROPERTY 를 두면 SKEL 재컴파일 시 부모·자식 이중 속성 충돌로 Ensure 나므로 이름을 분리한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SystemMessage", meta = (DisplayName = "Message Slot (바인드)"))
	TObjectPtr<UUserWidget> HudMessageSlotWidget;
};
