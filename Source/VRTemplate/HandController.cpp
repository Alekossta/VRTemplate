#include "HandController.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GrabbableStaticMeshComponent.h"

AHandController::AHandController()
{
	PrimaryActorTick.bCanEverTick = true;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MotionController"));
	SetRootComponent(MotionController);

	HandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandMesh"));
	HandMesh->SetupAttachment(MotionController);

	GripPoint = CreateDefaultSubobject<USceneComponent>(TEXT("GripPoint"));
	GripPoint->SetupAttachment(HandMesh);
}

void AHandController::BeginPlay()
{
	Super::BeginPlay();
}

void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	HandleVelocity();
}


void AHandController::Grab()
{
	if (!MotionController) return;

	FVector Direction = MotionController->GetTrackingSource() == EControllerHand::Left ? GetActorRightVector() : -GetActorRightVector();
	FVector SphereOrigin = GetActorLocation() + Direction * GrabOffset;
	float SphereRadius = GrabRadius;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;  // Array of object types to collide with
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_PhysicsBody));

	// Array to store the hit results
	TArray<FHitResult> OutHits;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());  // Ignore the owner of this actor (the player)

	bool bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
		GetWorld(),
		SphereOrigin,
		SphereOrigin,
		SphereRadius,
		ObjectTypes,
		false,  // bTraceComplex
		ActorsToIgnore,  // Ignore Actors (empty array in this case)
		EDrawDebugTrace::None,  // DrawDebugType (for visualization, optional)
		OutHits,
		true  // bIgnoreSelf
	);

	if (bHit)
	{

		// find component closest to the hand
		float ClosestDistance = 1000000.f;
		UGrabbableStaticMeshComponent* ClosestGrabbable = nullptr;
		for (auto& Hit : OutHits)
		{
			if (Hit.GetComponent() != nullptr)
			{
				UGrabbableStaticMeshComponent* Grabbable = Cast<UGrabbableStaticMeshComponent>(Hit.GetComponent());
				if (Grabbable)
				{
					float Distance = FVector::Dist(Hit.GetComponent()->GetComponentLocation(), GetActorLocation());
					if (Distance < ClosestDistance)
					{

						ClosestDistance = Distance;
						ClosestGrabbable = Grabbable;
					}
				}
			}
		}

		if (ClosestGrabbable)
		{
			GrabbedComponent = ClosestGrabbable;
			GrabbedComponent->GrabStart(this);
			bIsGrabbing = true;
			SetShowHand(false);
		}
	}
}

void AHandController::Release()
{
	if (bIsGrabbing)
	{
		if (GrabbedComponent != nullptr)
		{
			GrabbedComponent->GrabEnd(this);
			GrabbedComponent = nullptr;
			bIsGrabbing = false;
			SetShowHand(true);
		}
	}
}

void AHandController::SetTrackingSource(EControllerHand HandToSet)
{
	if (MotionController == nullptr) return;
	MotionController->SetTrackingSource(HandToSet);
}

void AHandController::SetShowHand(bool bShow)
{
	if (HandMesh)
	{
		HandMesh->SetVisibility(bShow);
	}
}

UMotionControllerComponent* AHandController::GetMotionControllerComp()
{
	return MotionController;
}

FVector AHandController::GetMotionVelocity() const
{
	return Velocity;
}

void AHandController::HandleVelocity()
{
	CurrentLocation = GetActorLocation();
	Velocity = (CurrentLocation - PrevLocation) / GetWorld()->GetDeltaSeconds();
	PrevLocation = CurrentLocation;
}

void AHandController::Activate()
{
	if (bIsGrabbing)
	{
		if (GrabbedComponent)
		{
			GrabbedComponent->ActivateStart(this);
		}
	}
}

void AHandController::Deactivate()
{
	if (bIsGrabbing)
	{
		if (GrabbedComponent)
		{
			GrabbedComponent->ActivateEnd(this);
		}
	}
}