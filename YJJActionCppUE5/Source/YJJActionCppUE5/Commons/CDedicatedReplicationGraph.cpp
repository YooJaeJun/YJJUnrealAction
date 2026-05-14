#include "Commons/CDedicatedReplicationGraph.h"

void UCDedicatedReplicationGraph::InitGlobalGraphNodes()
{
	Super::InitGlobalGraphNodes();

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
