#include "Commons/CPortal.h"
#include "Global.h"
#include "Characters/Player/CPlayableCharacter.h"
#include "Utilities/CLog.h"
#include "Utilities/YJJHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "TimerManager.h"

ACPortal::ACPortal()
{
	PrimaryActorTick.bCanEverTick = false;

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent_2"));
	SetRootComponent(OverlapSphere);
	OverlapSphere->InitSphereRadius(SphereRadius);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	OverlapSphere->SetGenerateOverlapEvents(true);

	PortalNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent_0"));
	PortalNiagara->SetupAttachment(OverlapSphere);
	PortalNiagara->SetAutoDestroy(false);
	PortalNiagara->SetRelativeRotation(FRotator(0.0f, 0.0f, 90.000237f));

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent_12"));
	PortalMesh->SetupAttachment(OverlapSphere);
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PortalMesh->SetGenerateOverlapEvents(false);
	PortalMesh->SetRelativeLocation(FVector::ZeroVector);
	PortalMesh->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
	PortalMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.01f));
	PortalMesh->SetMobility(EComponentMobility::Static);

	YJJHelpers::GetAsset<UStaticMesh>(
		&Mesh,
		TEXT("StaticMesh'/Engine/BasicShapes/Cylinder.Cylinder'"));
	YJJHelpers::GetAsset<UMaterialInterface>(
		&MeshMaterial,
		TEXT("Material'/Game/Materials/Portal/M_Portal.M_Portal'"));
	YJJHelpers::GetAsset<UNiagaraSystem>(
		&NiagaraSystem,
		TEXT("NiagaraSystem'/Game/Effects/Portal/NS_Portal.NS_Portal'"));

	YJJHelpers::GetClass<UUserWidget>(
		&LoadingWidgetClass,
		TEXT("WidgetBlueprint'/Game/Widgets/Loading/WB_Loading.WB_Loading_C'"));
}

void ACPortal::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyPortalVisualDefaults();
}

void ACPortal::ApplyPortalVisualDefaults()
{
	if (IsValid(OverlapSphere))
		OverlapSphere->SetSphereRadius(SphereRadius, false);

	if (IsValid(PortalMesh) && IsValid(Mesh))
		PortalMesh->SetStaticMesh(Mesh);

	if (IsValid(PortalMesh) && IsValid(MeshMaterial))
		PortalMesh->SetMaterial(0, MeshMaterial);

	if (IsValid(PortalNiagara) && IsValid(NiagaraSystem))
		PortalNiagara->SetAsset(NiagaraSystem, true);
}

void ACPortal::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(OverlapSphere))
		OverlapSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &ACPortal::OnSphereOverlapBegin);

	ApplyPortalVisualDefaults();

	if (bAutoActivateNiagara && IsValid(PortalNiagara))
		PortalNiagara->Activate(true);

	if (IsValid(LoopSound))
		UGameplayStatics::PlaySoundAtLocation(this, LoopSound, GetActorLocation());
}

void ACPortal::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GetWorld()))
		GetWorldTimerManager().ClearTimer(TravelDelayTimer);

	Super::EndPlay(EndPlayReason);
}

void ACPortal::OnSphereOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bTravelPending)
		return;

	if (false == IsValid(OtherActor))
	{
		CLog::Log(FString::Printf(TEXT("[Portal] Overlap 시 OtherActor 무효 — %s"), *GetName()));
		return;
	}

	const TObjectPtr<ACPlayableCharacter> playable = Cast<ACPlayableCharacter>(OtherActor);
	if (false == IsValid(playable))
		return;

	const TObjectPtr<APlayerController> playerController = Cast<APlayerController>(playable->GetController());
	if (false == IsValid(playerController))
	{
		CLog::Log(FString::Printf(TEXT("[Portal] PlayerController 없음 — %s"), *playable->GetName()));
		return;
	}

	if (false == playerController->IsLocalController())
		return;

	if (NextLevelName.IsNone())
	{
		CLog::Log(FString::Printf(TEXT("[Portal] NextLevelName 미설정 — %s"), *GetName()));
		return;
	}

	bTravelPending = true;

	if (IsValid(LoadingWidgetClass))
	{
		UUserWidget* loadingWidget = CreateWidget<UUserWidget>(playerController, LoadingWidgetClass);
		if (IsValid(loadingWidget))
			loadingWidget->AddToViewport(0);
		else
			CLog::Log(FString::Printf(TEXT("[Portal] WB_Loading 생성 실패 — %s"), *GetName()));
	}

	FTimerDelegate timerDel = FTimerDelegate::CreateUObject(this, &ACPortal::OnLoadingDelayElapsed);
	GetWorldTimerManager().SetTimer(TravelDelayTimer, timerDel, TravelDelaySeconds, false);
}

void ACPortal::OnLoadingDelayElapsed()
{
	if (false == IsValid(GetWorld()))
		return;

	if (NextLevelName.IsNone())
	{
		CLog::Log(FString::Printf(TEXT("[Portal] 지연 후에도 NextLevelName 없음 — %s"), *GetName()));
		bTravelPending = false;
		return;
	}

	UGameplayStatics::OpenLevel(this, NextLevelName, true);
}
