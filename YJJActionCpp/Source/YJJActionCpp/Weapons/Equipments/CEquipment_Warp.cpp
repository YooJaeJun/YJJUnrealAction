#include "Weapons/Equipments/CEquipment_Warp.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"

void UCEquipment_Warp::BeginPlay(ACCommonCharacter* InOwner, const FEquipmentData& InData)
{
	Super::BeginPlay(InOwner, InData);

	PlayerController = Cast<APlayerController>(Owner->MyCurController);

	//CHelpers::CreateActorComponent(this, &DecalComp, "MaterialInstanceConstant'/Game/Materials/M_Cursor_Inst.M_Cursor_Inst'");
}

void UCEquipment_Warp::Equip_Implementation()
{
	Super::Equip_Implementation();
}

void UCEquipment_Warp::Unequip_Implementation()
{
	Super::Unequip_Implementation();
}
