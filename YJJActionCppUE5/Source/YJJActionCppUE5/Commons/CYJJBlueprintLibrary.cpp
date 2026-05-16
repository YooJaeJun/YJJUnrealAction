#include "Commons/CYJJBlueprintLibrary.h"
#include "Utilities/YJJLocalizedText.h"
#include "Internationalization/Text.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundWave.h"

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
	float& Power,
	float& Launch,
	float& StopTime,
	USoundWave*& Sound,
	UFXSystemAsset*& Effect,
	FVector& EffectLocation,
	FVector& EffectScale,
	CEHitType& AttackType,
	TSubclassOf<UCameraShakeBase>& ShakeClass)
{
	Montage = HitData.Montage;
	PlayRate = HitData.PlayRate;
	Power = HitData.Power;
	Launch = HitData.Launch;
	StopTime = HitData.StopTime;
	Sound = HitData.Sound;
	Effect = HitData.Effect;
	EffectLocation = HitData.EffectLocation;
	EffectScale = HitData.EffectScale;
	AttackType = HitData.AttackType;
	ShakeClass = HitData.ShakeClass;
}
