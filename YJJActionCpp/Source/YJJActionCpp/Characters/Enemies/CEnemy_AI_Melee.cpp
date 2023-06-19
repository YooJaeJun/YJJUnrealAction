#include "Characters/Enemies/CEnemy_AI_Melee.h"
#include "Characters/AI/CAIController_Melee.h"

ACEnemy_AI_Melee::ACEnemy_AI_Melee()
{
	AIControllerClass = ACAIController_Melee::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}