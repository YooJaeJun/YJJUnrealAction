#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CAnimNotify_WeaponDispatchNotify.generated.h"

UCLASS(meta = (DisplayName = "End Dash Attack"))
class YJJACTIONCPPUE5_API UCAnimNotify_End_DashAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(meta = (DisplayName = "End Fall Down Attack"))
class YJJACTIONCPPUE5_API UCAnimNotify_End_FallDownAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(meta = (DisplayName = "Consume Stamina"))
class YJJACTIONCPPUE5_API UCAnimNotify_ConsumeStamina : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Consume Stamina")
	double StaminaAmount = 15.0;

	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};

UCLASS(meta = (DisplayName = "Box Collision"))
class YJJACTIONCPPUE5_API UCAnimNotify_BoxCollision : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual FString GetNotifyName_Implementation() const override;
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;
};
