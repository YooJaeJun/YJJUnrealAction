#include "Commons/CYJJBlueprintLibrary.h"
#include "Utilities/YJJLocalizedText.h"
#include "Internationalization/Text.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundBase.h"

FText UCYJJBlueprintLibrary::GetLocalizedUI(FName Key)
{
	return YJJLocalization::LocalizedText_From_UserInterface_Localization_Table_Key(Key);
}

void UCYJJBlueprintLibrary::BreakCCharacterInfo(
	const FCCharacterInfo& CharacterInfo,
	int32& Type,
	int32& Group,
	FName& Name,
	FLinearColor& BodyColor)
{
	Type = CharacterInfo.Type;
	Group = CharacterInfo.Group;
	Name = CharacterInfo.Name;
	BodyColor = CharacterInfo.BodyColor;
}

void UCYJJBlueprintLibrary::BreakHitData(
	const FHitData& HitData,
	UAnimMontage*& Montage,
	float& PlayRate,
	float& Damage,
	CEAttackType& AttackType,
	bool& bCanMove,
	float& Launch,
	CECrowdControl& CrowdControl,
	float& HitStop,
	USoundBase*& Sound,
	UFXSystemAsset*& Effect,
	FVector& EffectLocation,
	FVector& EffectScale,
	TSubclassOf<UCameraShakeBase>& ShakeClass)
{
	Montage = HitData.Montage;
	PlayRate = HitData.PlayRate;
	Damage = HitData.Damage;
	AttackType = HitData.AttackType;
	bCanMove = HitData.bCanMove;
	Launch = HitData.Launch;
	CrowdControl = HitData.CrowdControl;
	HitStop = HitData.HitStop;
	Sound = HitData.Sound;
	Effect = HitData.Effect;
	EffectLocation = HitData.EffectLocation;
	EffectScale = HitData.EffectScale;
	ShakeClass = HitData.ShakeClass;
}
