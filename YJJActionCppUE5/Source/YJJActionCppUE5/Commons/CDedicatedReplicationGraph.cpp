#include "Commons/CDedicatedReplicationGraph.h"
#include "Engine/ChildConnection.h"
#include "Engine/NetConnection.h"
#include "UObject/UObjectIterator.h"

FCDedicatedConnectionAlwaysRelevantNodePair::FCDedicatedConnectionAlwaysRelevantNodePair(
	UNetConnection* InConnection,
	UReplicationGraphNode_AlwaysRelevant_ForConnection* InNode)
	: NetConnection(InConnection)
	, Node(InNode)
{
}

bool FCDedicatedConnectionAlwaysRelevantNodePair::operator==(UNetConnection* InConnection) const
{
	return NetConnection == InConnection;
}

void UCDedicatedReplicationGraph::InitGlobalActorClassSettings()
{
	Super::InitGlobalActorClassSettings();

	for (TObjectIterator<UClass> classIt; classIt; ++classIt)
	{
		UClass* actorClass = *classIt;
		AActor* actorCDO = Cast<AActor>(actorClass->GetDefaultObject());
		if (false == IsValid(actorCDO) || false == actorCDO->GetIsReplicated())
			continue;

		if (actorClass->GetName().StartsWith(TEXT("SKEL_")) || actorClass->GetName().StartsWith(TEXT("REINST_")))
			continue;

		FClassReplicationInfo classInfo;
		classInfo.ReplicationPeriodFrame = GetReplicationPeriodFrameForFrequency(actorCDO->GetNetUpdateFrequency());

		if (actorCDO->bAlwaysRelevant || actorCDO->bOnlyRelevantToOwner)
			classInfo.SetCullDistanceSquared(0.0f);
		else
			classInfo.SetCullDistanceSquared(actorCDO->GetNetCullDistanceSquared());

		GlobalActorReplicationInfoMap.SetClassInfo(actorClass, classInfo);
	}
}

void UCDedicatedReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	// 월드 액터는 2D 그리드로 나눠 관심 영역 밖 복제 비용을 줄인다.
	GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
	GridNode->CellSize = 10000.0f;
	GridNode->SpatialBias = FVector2D(-UE_OLD_WORLD_MAX, -UE_OLD_WORLD_MAX);
	AddGlobalGraphNode(GridNode);

	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);
}

void UCDedicatedReplicationGraph::InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection)
{
	Super::InitConnectionGraphNodes(RepGraphConnection);

	UReplicationGraphNode_AlwaysRelevant_ForConnection* alwaysRelevantForConnectionNode =
		CreateNewNode<UReplicationGraphNode_AlwaysRelevant_ForConnection>();
	AddConnectionGraphNode(alwaysRelevantForConnectionNode, RepGraphConnection);

	AlwaysRelevantForConnectionList.Emplace(RepGraphConnection->NetConnection, alwaysRelevantForConnectionNode);
}

void UCDedicatedReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	if (false == IsValid(ActorInfo.Actor))
		return;

	if (ActorInfo.Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);

		return;
	}

	if (ActorInfo.Actor->bOnlyRelevantToOwner)
	{
		// 소유 커넥션이 아직 없으면 ServerReplicateActors에서 다시 connection node에 넣는다.
		ActorsWithoutNetConnection.AddUnique(ActorInfo.Actor);

		return;
	}

	if (ActorInfo.Actor->GetNetCullDistanceSquared() <= 0.0f)
	{
		// CullDistance 0 액터는 Grid 계산 경고를 피하기 위해 전역 노드로 둔다.
		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);

		return;
	}

	GridNode->AddActor_Dormancy(ActorInfo, GlobalInfo);
}

void UCDedicatedReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	if (false == IsValid(ActorInfo.Actor))
		return;

	if (ActorInfo.Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);
		SetActorDestructionInfoToIgnoreDistanceCulling(ActorInfo.GetActor());

		return;
	}

	if (ActorInfo.Actor->bOnlyRelevantToOwner)
	{
		UReplicationGraphNode_AlwaysRelevant_ForConnection* alwaysRelevantForConnectionNode =
			GetAlwaysRelevantNodeForConnection(ActorInfo.Actor->GetNetConnection());
		if (IsValid(alwaysRelevantForConnectionNode))
			alwaysRelevantForConnectionNode->NotifyRemoveNetworkActor(ActorInfo);

		ActorsWithoutNetConnection.Remove(ActorInfo.Actor);

		return;
	}

	if (ActorInfo.Actor->GetNetCullDistanceSquared() <= 0.0f)
	{
		AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);

		return;
	}

	GridNode->RemoveActor_Dormancy(ActorInfo);
}

void UCDedicatedReplicationGraph::RouteRenameNetworkActorToNodes(const FRenamedReplicatedActorInfo& ActorInfo)
{
	if (false == IsValid(ActorInfo.NewActorInfo.Actor))
		return;

	if (ActorInfo.NewActorInfo.Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyActorRenamed(ActorInfo);

		return;
	}

	if (ActorInfo.NewActorInfo.Actor->bOnlyRelevantToOwner)
	{
		UReplicationGraphNode_AlwaysRelevant_ForConnection* alwaysRelevantForConnectionNode =
			GetAlwaysRelevantNodeForConnection(ActorInfo.NewActorInfo.Actor->GetNetConnection());
		if (IsValid(alwaysRelevantForConnectionNode))
			alwaysRelevantForConnectionNode->NotifyActorRenamed(ActorInfo);

		return;
	}

	if (ActorInfo.NewActorInfo.Actor->GetNetCullDistanceSquared() <= 0.0f)
	{
		AlwaysRelevantNode->NotifyActorRenamed(ActorInfo);

		return;
	}

	GridNode->RenameActor_Dormancy(ActorInfo);
}

int32 UCDedicatedReplicationGraph::ServerReplicateActors(float DeltaSeconds)
{
	for (int32 index = ActorsWithoutNetConnection.Num() - 1; index >= 0; --index)
	{
		AActor* actor = ActorsWithoutNetConnection[index];
		if (false == IsValid(actor))
		{
			ActorsWithoutNetConnection.RemoveAtSwap(index);

			continue;
		}

		UNetConnection* netConnection = actor->GetNetConnection();
		if (false == IsValid(netConnection))
			continue;

		UReplicationGraphNode_AlwaysRelevant_ForConnection* alwaysRelevantForConnectionNode =
			GetAlwaysRelevantNodeForConnection(netConnection);
		if (IsValid(alwaysRelevantForConnectionNode))
		{
			alwaysRelevantForConnectionNode->NotifyAddNetworkActor(FNewReplicatedActorInfo(actor));
			ActorsWithoutNetConnection.RemoveAtSwap(index);
		}
	}

	return Super::ServerReplicateActors(DeltaSeconds);
}

UReplicationGraphNode_AlwaysRelevant_ForConnection* UCDedicatedReplicationGraph::GetAlwaysRelevantNodeForConnection(UNetConnection* Connection)
{
	if (false == IsValid(Connection))
		return nullptr;

	FCDedicatedConnectionAlwaysRelevantNodePair* nodePair = AlwaysRelevantForConnectionList.FindByKey(Connection);
	if (nullptr != nodePair && IsValid(nodePair->Node))
		return nodePair->Node;

	return nullptr;
}
