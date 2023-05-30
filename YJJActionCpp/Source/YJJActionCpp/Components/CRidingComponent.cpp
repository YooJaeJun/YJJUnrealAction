#include "Components/CRidingComponent.h"
#include "Global.h"
#include "Character/CCommonCharacter.h"
#include "Game/CGameMode.h"
#include "Widgets/CUserWidget_HUD.h"
#include "Widgets/Interaction/CUserWidget_Interaction.h"
#include "Animals/CAnimal_AI.h"
#include "Animation/AnimMontage.h"
#include "Engine/Texture2D.h"
#include "AIController.h"
#include "Player/CPlayableCharacter.h"

UCRidingComponent::UCRidingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = Cast<ACAnimal_AI>(GetOwner());
}

void UCRidingComponent::BeginPlay()
{
	Super::BeginPlay();

	TWeakObjectPtr<ACGameMode> gameMode = Cast<ACGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	Hud = Cast<UCUserWidget_HUD>(gameMode->GetHUD());

	if (!!Hud)
	{
		Hud->SetChild();
		Interaction = Hud->Interaction;

		CHelpers::LoadTextureFromPath<UTexture2D>(&InteractionKeyTexture, 
			TEXT("Texture2D'/Game/Assets/Textures/ButtonPrompts/F_Key_Dark.F_Key_Dark'"));
		Interaction->SetKeyIcon(InteractionKeyTexture);

		InteractionText = FText::FromString(TEXT("탑승"));

		Interaction->SetKeyIcon(InteractionKeyTexture);
		Interaction->SetText(InteractionText);
		Interaction->SetVisibility(ESlateVisibility::Collapsed);
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
	CheckNull(Interaction);

	TWeakObjectPtr<ACPlayableCharacter> player = Cast<ACPlayableCharacter>(OtherActor);
	CheckNull(player);

	Interaction->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCRidingComponent::EndOverlap(UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CheckNull(Interaction);

	TWeakObjectPtr<ACPlayableCharacter> player = Cast<ACPlayableCharacter>(OtherActor);
	CheckNull(player);

	Interaction->SetVisibility(ESlateVisibility::Collapsed);
}

void UCRidingComponent::Interact(ACCommonCharacter* InteractingActor)
{
}
