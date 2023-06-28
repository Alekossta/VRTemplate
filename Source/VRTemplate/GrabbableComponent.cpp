#include "GrabbableComponent.h"

UGrabbableComponent::UGrabbableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGrabbableComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UGrabbableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGrabbableComponent::Grab(class AHandController* Hand)
{
	GrabberHands.Add(Hand);
	OnGrabbed.Broadcast();
}

void UGrabbableComponent::Release(class AHandController* Hand)
{
	GrabberHands.Remove(Hand);
	if (GrabberHands.Num() == 0)
	{
		OnReleased.Broadcast();
	}
}