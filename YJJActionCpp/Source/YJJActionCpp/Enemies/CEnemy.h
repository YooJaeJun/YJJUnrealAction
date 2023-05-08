#pragma once

#include "CoreMinimal.h"
#include "Character/CCommonCharacter.h"
#include "Character/CInterface_CharacterAnim.h"
#include "Character/CInterface_CharacterBody.h"
#include "CEnemy.generated.h"

class UCWeaponComponent;
class UCMontagesComponent;

UCLASS()
class YJJACTIONCPP_API ACEnemy :
	public ACCommonCharacter,
	public ICInterface_CharacterAnim,
	public ICInterface_CharacterBody
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, Category = "Color")
		FLinearColor OriginColor = FLinearColor::White;

	UPROPERTY(VisibleAnywhere)
		class UCWeaponComponent* WeaponComponent;

	UPROPERTY(VisibleAnywhere)
		class UCMontagesComponent* MontagesComponent;

public:
	ACEnemy();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UFUNCTION()
		void OnStateTypeChanged(EStateType InPrevType, EStateType InNewType);

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;
};
