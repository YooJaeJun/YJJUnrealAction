#include "CPlayer.h"
#include "../Global.h"

ACPlayer::ACPlayer()
{
	TSharedPtr<UAction> Ptr;
	// TSharedPtr<UAction> Ptr2(MakeShared<UAction>());
	TWeakPtr<UAction> Ptr4;
	if (Ptr4.IsValid())
		UE_LOG(LogTemp, Warning, TEXT("Valid"));

	UE_LOG(LogTemp, Warning, TEXT("Log  "));

}

void ACPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

