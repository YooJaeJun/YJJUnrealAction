#pragma once

#include "CoreMinimal.h"
#include "ReplicationGraph.h"
#include "CDedicatedReplicationGraph.generated.h"

USTRUCT()
struct FCDedicatedConnectionAlwaysRelevantNodePair
{
	GENERATED_BODY()

	FCDedicatedConnectionAlwaysRelevantNodePair() { }
	FCDedicatedConnectionAlwaysRelevantNodePair(UNetConnection* InConnection, UReplicationGraphNode_AlwaysRelevant_ForConnection* InNode);

	bool operator==(UNetConnection* InConnection) const;

	UPROPERTY()
	TObjectPtr<UNetConnection> NetConnection = nullptr;

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_AlwaysRelevant_ForConnection> Node = nullptr;
};

UCLASS(Transient, Config = Engine)
class YJJACTIONCPPUE5_API UCDedicatedReplicationGraph : public UReplicationGraph
{
	GENERATED_BODY()

public:
	virtual void InitGlobalActorClassSettings() override;
	virtual void InitGlobalGraphNodes() override;
	virtual void InitConnectionGraphNodes(UNetReplicationGraphConnection* RepGraphConnection) override;
	virtual void RouteAddNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo, FGlobalActorReplicationInfo& GlobalInfo) override;
	virtual void RouteRemoveNetworkActorToNodes(const FNewReplicatedActorInfo& ActorInfo) override;
	virtual int32 ServerReplicateActors(float DeltaSeconds) override;

protected:
	virtual void RouteRenameNetworkActorToNodes(const FRenamedReplicatedActorInfo& ActorInfo) override;

private:
	UReplicationGraphNode_AlwaysRelevant_ForConnection* GetAlwaysRelevantNodeForConnection(UNetConnection* Connection);

private:
	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_GridSpatialization2D> GridNode;

	UPROPERTY()
	TObjectPtr<UReplicationGraphNode_ActorList> AlwaysRelevantNode;

	UPROPERTY()
	TArray<FCDedicatedConnectionAlwaysRelevantNodePair> AlwaysRelevantForConnectionList;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActorsWithoutNetConnection;
};
