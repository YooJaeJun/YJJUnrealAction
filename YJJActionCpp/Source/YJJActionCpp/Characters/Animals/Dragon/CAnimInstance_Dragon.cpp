#include "Characters/Animals/Dragon/CAnimInstance_Dragon.h"
#include "Characters/Animals/Dragon/CDragon_AI.h"

void UCAnimInstance_Dragon::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	OwnerDragon = Cast<ACDragon_AI>(Owner);
}

void UCAnimInstance_Dragon::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//if (OwnerDragon.IsValid() && 
	//	IsValid(OwnerDragon->Skill))
	//{
	//	Skill = OwnerDragon->Skill;

	//	if (Skill.IsValid())
	//		bRotating = Skill->bRotating;
	//}
}
