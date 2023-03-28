#pragma once
#include "CoreMinimal.h"

#define CheckTrue(x) { if (x == true) return; }
#define CheckTrue(x, y) { if (x == true) return y; }
