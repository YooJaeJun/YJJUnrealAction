#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_WeaponComboNotify.generated.h"

UCLASS(meta = (DisplayName = "Begin Air Combo"))
class YJJACTIONCPPUE5_API UCAnimNotify_Begin_AirCombo : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(meta = (DisplayName = "End Air Combo"))
class YJJACTIONCPPUE5_API UCAnimNotify_End_AirCombo : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(meta = (DisplayName = "Begin Flying Attack"))
class YJJACTIONCPPUE5_API UCAnimNotify_Begin_FlyingAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(meta = (DisplayName = "End Flying Attack"))
class YJJACTIONCPPUE5_API UCAnimNotify_End_FlyingAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(meta = (DisplayName = "End Down Attack"))
class YJJACTIONCPPUE5_API UCAnimNotify_End_DownAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
