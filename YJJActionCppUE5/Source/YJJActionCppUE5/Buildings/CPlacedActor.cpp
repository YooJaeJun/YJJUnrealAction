#include "Buildings/CPlacedActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ACPlacedActor::ACPlacedActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(false);
	NetDormancy = DORM_Awake;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(SceneRoot);
	MeshComp->SetCollisionProfileName(TEXT("BlockAll"));
}

void ACPlacedActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACPlacedActor, ItemID);
}

void ACPlacedActor::InitializePlacedActor(const FName InItemID)
{
	if (false == HasAuthority())
		return;

	ItemID = InItemID;
	FlushNetDormancy();
}

void ACPlacedActor::EnterPlacedDormancy()
{
	if (false == HasAuthority())
		return;

	// 설치 직후 초기 상태를 보낸 뒤에는 정적 액터로 보고 Dormancy 상태로 전환한다.
	SetNetDormancy(DORM_DormantAll);
}
