#include "HandController.h"
#include "MotionControllerComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GrabbableComponent.h"

AHandController::AHandController()
{
	PrimaryActorTick.bCanEverTick = true;

	MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("Motion Controller"));
	SetRootComponent(MotionController);

	HandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Hand Mesh"));
	HandMesh->SetupAttachment(MotionController);
}

void AHandController::BeginPlay()
{
	Super::BeginPlay();
}

void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHandController::TryGrab()
{
	FVector Direction = MotionController->GetTrackingSource() == EControllerHand::Left ? GetActorRightVector() : -GetActorRightVector();
	FVector SphereOrigin = GetActorLocation() + Direction * GrabOffset;
	float SphereRadius = GrabRadius;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;  // Array of object types to collide with
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));

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
		// find actor closest to the hand
		float ClosestDistance = 1000000.f;
		AActor* ClosestActor = nullptr;
		for (auto& Hit : OutHits)
		{
			if (Hit.GetActor() != nullptr)
			{
				float Distance = FVector::Dist(Hit.GetActor()->GetActorLocation(), GetActorLocation());
				if (Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
					ClosestActor = Hit.GetActor();
				}
			}
		}

		UGrabbableComponent* Grabbable = ClosestActor->FindComponentByClass<UGrabbableComponent>();
		if (Grabbable != nullptr)
		{
			Grab(Grabbable);
		}
	}
}

void AHandController::Grab(class UGrabbableComponent* Grabbed)
{
	if (Grabbed->IsOneHanded())
	{
		for (auto& Hand : Grabbed->GetGrabberHands())
		{
			Hand->Release();
		}
	}

	GrabbedComponent = Grabbed;
	GrabbedComponent->Grab(this);
	bIsGrabbing = true;
	SetShowHand(false);
}

void AHandController::Release()
{
	if (bIsGrabbing)
	{
		if (GrabbedComponent != nullptr)
		{	
			GrabbedComponent->Release(this);
			GrabbedComponent = nullptr;
			bIsGrabbing = false;
			SetShowHand(true);
		}
	}
}

void AHandController::SetTrackingSource (EControllerHand HandToSet)
{
	if (MotionController == nullptr) return;
	MotionController->SetTrackingSource(HandToSet);
}

void AHandController::SetShowHand(bool bShow)
{
	HandMesh->SetVisibility(bShow);
}

UMotionControllerComponent* AHandController::GetMotionControllerComp()
{
	return MotionController;
}
