#include "HandController.h"
#include "MotionControllerComponent.h"

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
