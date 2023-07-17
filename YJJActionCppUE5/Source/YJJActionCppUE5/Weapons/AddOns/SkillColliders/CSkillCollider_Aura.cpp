#include "Weapons/AddOns/SkillColliders/CSkillCollider_Aura.h"
#include "Global.h"
#include "NiagaraComponent.h"
#include "Characters/CCommonCharacter.h"
#include "Components/BoxComponent.h"

ACSkillCollider_Aura::ACSkillCollider_Aura()
{
	YJJHelpers::CreateComponent<USceneComponent>(this, &Root, "Root");
	YJJHelpers::CreateComponent<UNiagaraComponent>(this, &Niagara, "Niagara", Root.Get());
	YJJHelpers::CreateComponent<UBoxComponent>(this, &Box, "Box", Root.Get());

	TObjectPtr<UNiagaraSystem> niagara;
	YJJHelpers::GetAsset<UNiagaraSystem>(&niagara, "/Script/Niagara.NiagaraSystem'/Game/Assets/Effects/sA_StylizedSwordSet/Fx/NS_Ulti_lv1.NS_Ulti_lv1'");
	Niagara->SetAsset(niagara);
}

void ACSkillCollider_Aura::BeginPlay()
{
	Super::BeginPlay();

	Niagara->SetNiagaraVariableObject("Mesh_Scale", this);
	Niagara->OnSystemFinished.AddDynamic(this, &ACSkillCollider_Aura::OnSystemFinished);

	Box->OnComponentBeginOverlap.AddDynamic(this, &ACSkillCollider_Aura::OnComponentBeginOverlap);
	Box->OnComponentEndOverlap.AddDynamic(this, &ACSkillCollider_Aura::OnComponentEndOverlap);

	const FTimerDelegate timerDelegate = FTimerDelegate::CreateLambda([this]()
		{
			for (int32 i = Hitted.Num() - 1; i >= 0; i--)
				HitData.SendDamage(
					Cast<ACCommonCharacter>(GetOwner()),
					this,
					Hitted[i]);
		});

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		timerDelegate,
		DamageInterval,
		true,
		0);
}


void ACSkillCollider_Aura::OnSystemFinished(UNiagaraComponent* PSystem)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

	Destroy();
}

//void ACSkillCollider_Aura::ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& ParticleData, UNiagaraSystem* NiagaraSystem)
//{
//	Box->SetRelativeScale3D(ParticleData[0].Position);
//
//	FVector location = Box->GetScaledBoxExtent();
//	// 위치 조정 위함. Mesh 90도 회전되었으니 Y축
//	location.Y = 0;
//
//	Box->SetRelativeLocation(location);
//}

void ACSkillCollider_Aura::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CheckTrue(GetOwner() == OtherActor);

	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	if (character.IsValid())
		Hitted.AddUnique(character.Get());
}

void ACSkillCollider_Aura::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CheckTrue(GetOwner() == OtherActor);

	const TWeakObjectPtr<ACCommonCharacter> character = Cast<ACCommonCharacter>(OtherActor);
	if (character.IsValid())
		Hitted.Remove(character.Get());
}