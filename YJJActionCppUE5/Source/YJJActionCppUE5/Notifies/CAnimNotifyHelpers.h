#pragma once

#include "CoreMinimal.h"

class USkeletalMeshComponent;
class UCWeaponComponent;

/** MeshComp 소유 액터에서 UCWeaponComponent 를 찾는다. 애니 노티 공통 헬퍼. */
YJJACTIONCPPUE5_API UCWeaponComponent* CAnimNotifyHelpers_ResolveWeaponComponent(
	USkeletalMeshComponent* MeshComp);
