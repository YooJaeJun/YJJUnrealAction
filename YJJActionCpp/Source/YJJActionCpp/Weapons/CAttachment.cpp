#include "Weapons/CAttachment.h"
#include "Global.h"
#include "Characters/CCommonCharacter.h"
#include "Components/SceneComponent.h"
#include "Components/ShapeComponent.h"

ACAttachment::ACAttachment()
{
	YJJHelpers::CreateComponent(this, &Root, "Root");
}

void ACAttachment::BeginPlay()
{
	Owner = Cast<ACCommonCharacter>(GetOwner());

	TArray<USceneComponent*> children;
	Root->GetChildrenComponents(true, children);

	for (USceneComponent* child : children)
	{
		UShapeComponent* shape = Cast<UShapeComponent>(child);

		if (IsValid(shape))
		{
			shape->OnComponentBeginOverlap.AddUniqueDynamic(this, &ACAttachment::OnComponentBeginOverlap);
			shape->OnComponentEndOverlap.AddUniqueDynamic(this, &ACAttachment::OnComponentEndOverlap);

			Collisions.Add(shape);
		}
	}

	OffCollisions();

	Super::BeginPlay();
}

void ACAttachment::AttachTo(FName InSocketName)
{
	AttachToComponent(Owner->GetMesh(),
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), 
		InSocketName);
}

void ACAttachment::AttachToCollision(FName InCollisionName)
{
	for (UShapeComponent* collision : Collisions)
	{
		if (collision->GetName() == InCollisionName.ToString())
		{
			collision->AttachToComponent(Owner->GetMesh(),
				FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), 
				InCollisionName);

			return;
		}
	}
}

void ACAttachment::DetachTo(FName InSocketName)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
}

void ACAttachment::DetachToCollision(FName InCollisionName)
{
	for (UShapeComponent* collision : Collisions)
	{
		if (collision->GetName() == InCollisionName.ToString())
			collision->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void ACAttachment::OnComponentBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	CheckTrue(Owner == OtherActor);
	CheckTrue(Owner->GetClass() == OtherActor->GetClass());

	if (OnAttachmentBeginOverlap.IsBound())
		OnAttachmentBeginOverlap.Broadcast(
			Owner.Get(), 
			this, 
			Cast<ACCommonCharacter>(OtherActor));
}

void ACAttachment::OnComponentEndOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex)
{
	CheckTrue(Owner == OtherActor);
	CheckTrue(Owner->GetClass() == OtherActor->GetClass());

	if (OnAttachmentEndOverlap.IsBound())
		OnAttachmentEndOverlap.Broadcast(Owner.Get(), Cast<ACCommonCharacter>(OtherActor));
}

void ACAttachment::OnCollisions()
{
	//Pre
	if (OnAttachmentBeginCollision.IsBound())
		OnAttachmentBeginCollision.Broadcast();

	for (UShapeComponent* shape : Collisions)
		shape->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	//Post
}

void ACAttachment::OffCollisions()
{
	//Pre
	if (OnAttachmentEndCollision.IsBound())
		OnAttachmentEndCollision.Broadcast();

	for (UShapeComponent* shape : Collisions)
		shape->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Post
}