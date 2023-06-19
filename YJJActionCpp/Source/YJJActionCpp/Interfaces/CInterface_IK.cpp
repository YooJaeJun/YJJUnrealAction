#include "Interfaces/CInterface_IK.h"

void ICInterface_IK::SetLegIKAlpha(const float InAlpha)
{
	LegIKAlpha = InAlpha;
}

float ICInterface_IK::GetLegIKAlpha() const
{
	return LegIKAlpha;
}
