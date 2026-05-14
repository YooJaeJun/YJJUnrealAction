#include "Commons/CYJJBlueprintLibrary.h"
#include "Animation/AnimMontage.h"
#include "Sound/SoundWave.h"

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
	CEHitType& AttackType)
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
}
