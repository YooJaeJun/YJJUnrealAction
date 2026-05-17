#include "Characters/Animals/Dragon/CAnimInstance_Dragon.h"
#include "Characters/Animals/Dragon/CDragon.h"

void UCAnimInstance_Dragon::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	OwnerDragon = Cast<ACDragon>(Owner);
}

void UCAnimInstance_Dragon::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
}
