#include "Characters/AI/CAIController.h"

#include "Global.h"

#include "Characters/Animals/CAnimal.h"

#include "Characters/Enemies/CEnemy.h"

#include "BehaviorTree/BehaviorTree.h"

#include "BehaviorTree/BehaviorTreeComponent.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "BehaviorTree/BlackboardData.h"

#include "Components/CCharacterInfoComponent.h"


#include "Perception/AIPerceptionComponent.h"

#include "Perception/AISense_Sight.h"

#include "Perception/AISenseConfig_Hearing.h"

#include "Perception/AISenseConfig_Sight.h"



const FName ACAIController::SelfActor(TEXT("SelfActor"));

const FName ACAIController::Target(TEXT("Target"));

const FName ACAIController::Behavior(TEXT("Behavior"));

const FName ACAIController::TargetLocation(TEXT("TargetLocation"));

bool ACAIController::ShouldUseHearingPerceptionSense() const
{
	return true;
}

void ACAIController::ConfigureSightSenseAffiliation(UAISenseConfig_Sight& SightConfigRef)
{
	SightConfigRef.DetectionByAffiliation.bDetectEnemies = true;
	SightConfigRef.DetectionByAffiliation.bDetectFriendlies = true;
	SightConfigRef.DetectionByAffiliation.bDetectNeutrals = true;
}

ACAIController::ACAIController(const FObjectInitializer& ObjectInitializer)

	: Super(ObjectInitializer)

{
	// TransformComponent0 + 절대 회전 Root 는 엔진 AController ctor 에서 이미 생성·설정됨 — 여기서 다시 만들면 서브오브젝트 이름 충돌로 Fatal 난다.

	// 블프 BB/BT 는 RunAI 진입 시 GetAssetDynamic 으로만 로드(ConstructorHelpers 순환 교착 방지).


	// AAIController ctor 는 UAIPerceptionComponent 를 만들지 않으며, PerceptionComponent 포인터는 PostRegisterAllComponents 에서 FindComponent 로만 채워진다.
	// ctor 시점에는 PerceptionComponent 가 비어 있으므로 먼저 서브오브젝트를 만들고 해당 포인터로만 설정한다.(ConfigureSense 안의 멤버 순회는 유효 this 가 필요.)

	UAIPerceptionComponent* perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

	if (!IsValid(perception))
	{
		CLog::Log(FString::Printf(TEXT("ACAIController: UAIPerceptionComponent 생성 실패 — 지각 초기화 생략 [%s]"),

			*GetClass()->GetName()));

		return;
	}

	YJJHelpers::CreateActorComponent<UAISenseConfig_Sight>(this, &SightConfig, TEXT("SenseSight"));

	ConfigureSightSenseAffiliation(*SightConfig);

	SightConfig->SightRadius = SightRadius;

	SightConfig->LoseSightRadius = LoseSightRadius;

	SightConfig->PeripheralVisionAngleDegrees = Angle;

	SightConfig->SetMaxAge(MaxAge);

	if (ShouldUseHearingPerceptionSense())
	{
		YJJHelpers::CreateActorComponent<UAISenseConfig_Hearing>(this, &HearingConfig, TEXT("SenseHearing"));

		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;

		HearingConfig->SetMaxAge(MaxAge);
	}

	perception->ConfigureSense(*SightConfig);

	if (ShouldUseHearingPerceptionSense())
	{
		perception->ConfigureSense(*HearingConfig);
	}

	perception->SetDominantSense(SightConfig->GetSenseImplementation());

	// BP_Event: OnPerceptionUpdated -> GetCurrentlyPerceivedActors 루프로 Target 갱신.

	perception->OnPerceptionUpdated.AddDynamic(this, &ACAIController::OnPerceptionUpdated);
}



void ACAIController::OnPossess(APawn* InPawn)

{

	Super::OnPossess(InPawn);

	RunAI(InPawn);

}



void ACAIController::OnUnPossess()

{

	EnemyAI = nullptr;

	Super::OnUnPossess();

}



void ACAIController::AiControllerEnsureDefaultBlackboardAndBehaviorTreeLoadedIfUnset()
{
	if (false == IsValid(BBAsset))
	{
		YJJHelpers::GetAssetDynamic<UBlackboardData>(
			&BBAsset,
			FString(TEXT("/Script/AIModule.BlackboardData'/Game/Character/Enemies/BB_Enemy.BB_Enemy'")));
	}
	if (false == IsValid(BTAsset))
	{
		YJJHelpers::GetAssetDynamic<UBehaviorTree>(
			&BTAsset,
			FString(TEXT("/Script/AIModule.BehaviorTree'/Game/Character/Enemies/Melee/CBT_Melee.CBT_Melee'")));
	}
}



void ACAIController::RunAI(APawn* InPossessedPawn)

{

	AiControllerEnsureDefaultBlackboardAndBehaviorTreeLoadedIfUnset();


	UBehaviorTree* effectiveBT = BTAsset.Get();



	ACEnemy* const possessedEnemyAi = Cast<ACEnemy>(InPossessedPawn);

	if (nullptr != possessedEnemyAi)

	{

		EnemyAI = possessedEnemyAi;

		UBehaviorTree* const enemyBt = possessedEnemyAi->GetEnemyBehaviorTreeForController();

		if (IsValid(enemyBt))

		{

			effectiveBT = enemyBt;

		}

	}

	else

	{

		EnemyAI = nullptr;

	}



	const ACAnimal* const animalAi = Cast<ACAnimal>(InPossessedPawn);

	if (nullptr != animalAi)

	{

		UBehaviorTree* const pawnBt = animalAi->GetAnimalBehaviorTreeForController();

		if (IsValid(pawnBt))

		{

			effectiveBT = pawnBt;

		}

	}



	if ((nullptr != effectiveBT) && UseBlackboard(BBAsset, static_cast<UBlackboardComponent*&>(Blackboard)))

	{

		RunBehaviorTree(effectiveBT);

	}

}



void ACAIController::StopAI() const

{

	const TWeakObjectPtr<UBehaviorTreeComponent> behaviorTreeComponent =

		Cast<UBehaviorTreeComponent>(BrainComponent);



	CheckNull(behaviorTreeComponent);



	behaviorTreeComponent->StopTree(EBTStopMode::Safe);

}



void ACAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)

{

	// 델리게이트 시그니처상 전달되는 배열 대신 현재 관측 집합을 다시 묻는다(BP_Perception 과 동일 흐름).

	(void)UpdatedActors;



	ProcessPerceptionAndUpdateBlackboardTarget();

}



void ACAIController::ProcessPerceptionAndUpdateBlackboardTarget()

{

	APawn* const controlledPawn = GetPawn();

	if (false == IsValid(controlledPawn))

	{

		return;

	}



	// Possess 된 폰이 ACCommonCharacter 가 아니면 그룹/블랙보드 동기 경로 없음(BP 에서도 BP_Character 캐스트).

	ACCommonCharacter* const controlledCharacter = Cast<ACCommonCharacter>(controlledPawn);

	if (nullptr == controlledCharacter)

	{

		return;

	}



	const TWeakObjectPtr<UCCharacterInfoComponent> infoComp =

		YJJHelpers::GetComponent<UCCharacterInfoComponent>(controlledCharacter);

	if (false == infoComp.IsValid())

	{

		CLog::Log(FString::Printf(

			TEXT("ACAIController: CharacterInfoComp 없음 — Possess 대상 검사 불가 [%s → %s]"),

			*GetName(),

			*controlledPawn->GetName()));

		return;

	}



	if (false == IsValid(Blackboard))

	{

		CLog::Log(FString::Printf(

			TEXT("ACAIController: Blackboard 미초기화 — 지각 업데이트 무시 [%s]"),

			*GetName()));

		return;

	}



	if (false == IsValid(PerceptionComponent))

	{

		CLog::Log(FString::Printf(TEXT("ACAIController: PerceptionComponent 없음 [%s]"), *GetName()));

		return;

	}



	TArray<AActor*> perceivedActors;



	TSubclassOf<UAISense> senseClass;

	if (IsValid(SightConfig.Get()))

	{

		senseClass = SightConfig->GetSenseImplementation();

	}



	if (senseClass != nullptr)

	{

		PerceptionComponent->GetCurrentlyPerceivedActors(senseClass, perceivedActors);

	}

	else

	{

		// SightConfig 가 비정상일 때 우회(지배 감각이 Sight 인 전제).

		PerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), perceivedActors);

	}



	for (AActor* const perceivedActor : perceivedActors)

	{

		ACCommonCharacter* const perceivedCharacter = Cast<ACCommonCharacter>(perceivedActor);

		if (nullptr == perceivedCharacter)

		{

			continue;

		}



		// 레거시 BF_Helpers::IsSameGroup 와 동일하게 동료는 스킵, 첫 비동료만 Target 에 넣고 루프 종료(BP 의 ForEachWithBreak).

		if (true == infoComp->IsSameGroup(perceivedCharacter))

		{

			continue;

		}



		Blackboard->SetValueAsObject(Target, perceivedCharacter);

		break;

	}

}


