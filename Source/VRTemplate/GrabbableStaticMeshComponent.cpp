// Fill out your copyright notice in the Description page of Project Settings.


#include "GrabbableStaticMeshComponent.h"
#include "HandController.h"

UGrabbableStaticMeshComponent::UGrabbableStaticMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UGrabbableStaticMeshComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bShouldAttach)
	{
		if (bShouldSimulatePhysics)
		{
			SetSimulatePhysics(true);
			SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		}
		else
		{
			SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SetCollisionResponseToAllChannels(ECR_Overlap);
		}
	}
}

void UGrabbableStaticMeshComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UGrabbableStaticMeshComponent::GrabStart(AHandController* Hand)
{
	if (bIsOneHanded)
	{
		for (auto GrabberHand : GrabberHands)
		{
			GrabberHand->Release();
		}

		if (bShouldAttach)
		{
			if (bShouldSimulatePhysics)
			{
				SetSimulatePhysics(false);
				SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}

			FAttachmentTransformRules AttachRules = FAttachmentTransformRules::KeepWorldTransform;
			if (bShouldSnap)
			{
				AttachRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
			}

			GetOwner()->AttachToComponent(Hand->GetGripPoint(), AttachRules);
		}
	}

	GrabberHands.Add(Hand);
	OnGrabStart.Broadcast(Hand);
}

void UGrabbableStaticMeshComponent::GrabEnd(AHandController* Hand)
{
	GrabberHands.Remove(Hand);

	if (GrabberHands.Num() == 0)
	{
		OnGrabEnd.Broadcast(Hand);
		ActivateEnd(Hand);
	}


	if (bShouldAttach)
	{
		FDetachmentTransformRules DetachRules = FDetachmentTransformRules::KeepWorldTransform;
		GetOwner()->DetachFromActor(DetachRules);

		if (bShouldSimulatePhysics)
		{
			SetSimulatePhysics(true);
			SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			AddImpulse(Hand->GetMotionVelocity() * Hand->GetHandStrength());
		}
	}
}

void UGrabbableStaticMeshComponent::ActivateStart(AHandController* Hand)
{
	OnActivateStart.Broadcast(Hand);
}

void UGrabbableStaticMeshComponent::ActivateEnd(AHandController* Hand)
{
	OnActivateEnd.Broadcast(Hand);
}

void UGrabbableStaticMeshComponent::ForceDrop()
{
	for (auto GrabberHand : GrabberHands)
	{
		GrabberHand->Release();
	}
}

