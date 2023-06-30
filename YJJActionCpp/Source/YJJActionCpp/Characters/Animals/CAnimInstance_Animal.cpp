#include "Characters/Animals/CAnimInstance_Animal.h"
#include "Global.h"

void UCAnimInstance_Animal::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	CheckNull(Owner.Get());

	bRiding = Owner->GetbRiding();
}