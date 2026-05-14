#include "Commons/CDedicatedReplicationGraph.h"

void UCDedicatedReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

	// 월드 액터는 2D 그리드로 나눠 관심 영역 밖 복제 비용을 줄인다.
	GridNode = CreateNewNode<UReplicationGraphNode_GridSpatialization2D>();
	GridNode->CellSize = 10000.0f;
	GridNode->SpatialBias = FVector2D(-HALF_WORLD_MAX, -HALF_WORLD_MAX);
	AddGlobalGraphNode(GridNode);

	AlwaysRelevantNode = CreateNewNode<UReplicationGraphNode_ActorList>();
	AddGlobalGraphNode(AlwaysRelevantNode);
}

void UCDedicatedReplicationGraph::RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo)
{
	if (false == IsValid(ActorInfo.Actor))
		return;

	// GameState처럼 항상 필요한 액터는 거리 컬링 대상에서 제외한다.
	if (ActorInfo.Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyAddNetworkActor(ActorInfo);

		return;
	}

	GridNode->NotifyAddNetworkActor(ActorInfo);
}

void UCDedicatedReplicationGraph::RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo)
{
	if (false == IsValid(ActorInfo.Actor))
		return;

	if (ActorInfo.Actor->bAlwaysRelevant)
	{
		AlwaysRelevantNode->NotifyRemoveNetworkActor(ActorInfo);

		return;
	}

	GridNode->NotifyRemoveNetworkActor(ActorInfo);
}
