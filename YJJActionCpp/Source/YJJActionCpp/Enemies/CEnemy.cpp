#include "Enemies/CEnemy.h"
#include "Global.h"
#include "Character/CAnimInstance_Human.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CWeaponComponent.h"
#include "Components/CMontagesComponent.h"
#include "Components/CMovementComponent.h"

ACEnemy::ACEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	CHelpers::CreateActorComponent<UCWeaponComponent>(this, &WeaponComponent, "Weapon");
	CHelpers::CreateActorComponent<UCMontagesComponent>(this, &MontagesComponent, "Montages");
	CHelpers::CreateActorComponent<UCMovementComponent>(this, &MovementComponent, "Movement");
	CHelpers::CreateActorComponent<UCStateComponent>(this, &StateComponent, "State");

	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	USkeletalMesh* mesh;
	CHelpers::GetAsset<USkeletalMesh>(&mesh, "SkeletalMesh'/Game/Assets/Enemies/Skeleton_archer/mesh/SK_Skeleton_archer.SK_Skeleton_archer'");
	GetMesh()->SetSkeletalMesh(mesh);

	TSubclassOf<UCAnimInstance_Human> animInstance;
	CHelpers::GetClass<UCAnimInstance_Human>(&animInstance, "AnimBlueprint'/Game/Character/ABP_CHuman.ABP_CHuman_C'");
	GetMesh()->SetAnimClass(animInstance);

	GetCharacterMovement()->RotationRate = FRotator(0, 720, 0);
}

void ACEnemy::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent->InputAction_Run();

	Create_DynamicMaterial(this);
	Change_Color(this, OriginColor);

	StateComponent->OnStateTypeChanged.AddDynamic(this, &ACEnemy::OnStateTypeChanged);
}

void ACEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ACEnemy::OnStateTypeChanged(EStateType InPrevType, EStateType InNewType)
{
}

float ACEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	float damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CLog::Print(damage);

	return damage;
}
