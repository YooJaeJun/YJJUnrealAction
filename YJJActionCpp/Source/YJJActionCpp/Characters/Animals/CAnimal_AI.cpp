#include "Characters/Animals/CAnimal_AI.h"
#include "Characters/AI/CAIController_Melee.h"

ACAnimal_AI::ACAnimal_AI()
{
	AIControllerClass = ACAIController_Melee::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
