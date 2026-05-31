#include "Weapons/AnimInstances/CAnimInstance_Bow.h"

void UCAnimInstance_Bow::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// UObject 수명은 AnimInstance가 관리한다. TSharedPtr은 외부(Act·Skill)에서 Bend를 갱신하기 위한 핸들만 제공한다.
	BendShared = MakeShareable(&Bend, [](float*) {});
}

TSharedPtr<float> UCAnimInstance_Bow::GetBend()
{
	if (false == BendShared.IsValid())
	{
		BendShared = MakeShareable(&Bend, [](float*) {});
	}

	return BendShared;
}
