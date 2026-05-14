#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "CDedicatedReplicationGraph.generated.h"

UCLASS(Transient, Config = Engine)
class YJJACTIONCPPUE5_API UCDedicatedReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	virtual void InitGlobalGraphNodes() override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;

private:
	UPROPERTY()
		TObjectPtr<UReplicationGraphNode_GridSpatialization2D> GridNode;

	UPROPERTY()
		TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantNode;
};
