#include "Components/CRidingComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Game/CGameMode.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"

UCRidingComponent::UCRidingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACCommonCharacter>(GetOwner());
}

void UCRidingComponent::BeginPlay()
{
	Super::BeginPlay();

	TWeakObjectPtr<ACGameMode> gameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(Owner->GetWorld()));
	Hud = Cast<UCUserWidget_HUD>(gameMode->GetHUD());

	if (!!Hud)
	{
		Hud->SetChild();
		Interaction = Hud->Interaction;
	}
}

void UCRidingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCRidingComponent::BeginOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!!Interaction)
		Interaction->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCRidingComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!!Interaction)
		Interaction->SetVisibility(ESlateVisibility::Collapsed);
}

void UCRidingComponent::Interact(ACCommonCharacter* InteractingActor)
{

}

