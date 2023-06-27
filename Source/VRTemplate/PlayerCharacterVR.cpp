#include "PlayerCharacterVR.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HandController.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "HeadMountedDisplayFunctionLibrary.h"

APlayerCharacterVR::APlayerCharacterVR()
{
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera->SetupAttachment(VRRoot);

	Orientation = CreateDefaultSubobject<UArrowComponent>(TEXT("Orientation Arrow"));
	Orientation->SetupAttachment(GetRootComponent());

	TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("Teleport Path"));
	TeleportPath->SetupAttachment(GetRootComponent());

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Destination Marker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	TeleportProjectileRadius = 5;
	TeleportProjectileSpeed = 800;
	TeleportSimulationTime = 2;
	TeleportProjectionExtent = FVector(10.f, 10.f, 10.f);

	MovementWay = EMovementWay::Both;

	DegreesToRotate = 45.f;

	// So that the character follows the rotation of the controller (only in yaw)
	bUseControllerRotationYaw = true;

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = 450.f;
	}
}

void APlayerCharacterVR::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);
	HandTryingToTeleport = EControllerHand::AnyHand;

	// Hide all teleport path spline meshes
	for (USplineMeshComponent* SplineMesh : TeleportPathMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}

	// Not really sure about this one

	// Setup vr root / camera based on selected pose (sitting/standing)
	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	if (Pose == EPose::Standing)
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);
		VRRoot->SetRelativeLocation(FVector(0, 0, -CapsuleHalfHeight));
	}
	else
	{
		UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Eye);
		VRRoot->SetRelativeLocation(FVector(0, 0, (CapsuleHalfHeight * 2) - CapsuleHalfHeight / 2));
	}
}

void APlayerCharacterVR::SpawnHands()
{
	HandsOwner = VRRoot;
	Super::SpawnHands();
}

void APlayerCharacterVR::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Started, this, &APlayerCharacterVR::Rotate);
		EnhancedInputComponent->BindAction(TeleportLeftAction, ETriggerEvent::Started, this, &APlayerCharacterVR::ThumbstickPressedLeft);
		EnhancedInputComponent->BindAction(TeleportLeftAction, ETriggerEvent::Completed, this, &APlayerCharacterVR::ThumbstickReleasedLeft);
		EnhancedInputComponent->BindAction(TeleportRightAction, ETriggerEvent::Started, this, &APlayerCharacterVR::ThumbstickPressedRight);
		EnhancedInputComponent->BindAction(TeleportRightAction, ETriggerEvent::Completed, this, &APlayerCharacterVR::ThumbstickReleasedRight);
	}
}

void APlayerCharacterVR::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	KeepCameraAndCapsuleTogether();
	UpdateDestinationMarker();
}

void APlayerCharacterVR::KeepCameraAndCapsuleTogether()
{
	if (Camera == nullptr || GetArrowComponent() == nullptr) return; // Do some pointer checks

	// Make arrow of character to follow camera z axis rotation so that we have an orientation reference
	float RotationYaw = Camera->GetComponentRotation().Yaw;
	FRotator Rotation = FRotator(Orientation->GetComponentRotation().Pitch, RotationYaw,
		Orientation->GetComponentRotation().Roll);
	Orientation->SetWorldRotation(Rotation);

	FVector CameraOffsetPerFrame = Camera->GetComponentLocation() - GetActorLocation();
	CameraOffsetPerFrame.Z = 0; // Dont align z cause the capsule would move up and down with the headset
	AddActorWorldOffset(CameraOffsetPerFrame, true);
	if (VRRoot == nullptr) return;
	VRRoot->AddWorldOffset(-CameraOffsetPerFrame, true);
}

void APlayerCharacterVR::FinishTeleport()
{
	DestinationMarker->SetVisibility(false);
	TArray<FVector> EmptyPath;

	DrawTeleportPath(EmptyPath);
	bCanTeleport = false;

	HandControllerLeft->SetShowHand(true);
	HandControllerRight->SetShowHand(true);
}


void APlayerCharacterVR::Teleport()
{
	FVector Destination = DestinationMarker->GetComponentLocation();
	Destination += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();
	FHitResult* Hit = nullptr;
	SetActorLocation(Destination, false, Hit, ETeleportType::ResetPhysics);

	FinishTeleport();
}

void APlayerCharacterVR::UpdateDestinationMarker()
{
	// dont't update the destination marker if not needed
	if (MovementWay == EMovementWay::Walking || HandTryingToTeleport == EControllerHand::AnyHand) return;

	TArray<FVector> PathToDestination;
	FVector TeleportDestination;
	bool bHasDestination = FindTeleportDestination(PathToDestination, TeleportDestination);

	if (bHasDestination)
	{
		DestinationMarker->SetWorldLocation(TeleportDestination);
		DestinationMarker->SetVisibility(true);

		DrawTeleportPath(PathToDestination);
		bCanTeleport = true;
	}
	else
	{
		FinishTeleport();
	}
}

bool APlayerCharacterVR::FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation)
{
	FVector Start;
	FVector Direction;
	FVector End;
	switch (HandTryingToTeleport)
	{
		case EControllerHand::Left:
		{
			Start = HandControllerLeft->GetActorLocation();
			End = HandControllerLeft->GetActorLocation();
			Direction = HandControllerLeft->GetActorForwardVector();
			HandControllerLeft->SetShowHand(false);
			HandControllerRight->SetShowHand(true);
			break;
		}
		case EControllerHand::Right:
		{
			Start = HandControllerRight->GetActorLocation();
			End = HandControllerRight->GetActorLocation();
			Direction = HandControllerRight->GetActorForwardVector();
			HandControllerRight->SetShowHand(false);
			HandControllerLeft->SetShowHand(true);
			break;
		}
	}

	FPredictProjectilePathParams Params = FPredictProjectilePathParams(TeleportProjectileRadius, Start, Direction * TeleportProjectileSpeed,
		TeleportSimulationTime, ECC_GameTraceChannel8, this);

	FPredictProjectilePathResult Result;
	bool bHit = UGameplayStatics::PredictProjectilePath(this, Params, Result);

	if (!bHit) return false;

	for (FPredictProjectilePathPointData PointData : Result.PathData)
	{
		OutPath.Add(PointData.Location);
	}

	FNavLocation NavLocation;
	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(Result.HitResult.Location,
		NavLocation, TeleportProjectionExtent);

	if (!bOnNavMesh) return false;

	OutLocation = NavLocation.Location;

	return true;
}

void APlayerCharacterVR::DrawTeleportPath(const TArray<FVector>& Path)
{
	UpdateSpline(Path);

	for (USplineMeshComponent* SplineMesh : TeleportPathMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}

	for (int32 i = 0; i < Path.Num(); ++i)
	{
		if (TeleportPathMeshPool.Num() <= i)
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStaticMesh(TeleportArchMesh);
			SplineMesh->SetMaterial(0, TeleportArchMaterial);
			SplineMesh->RegisterComponent();

			TeleportPathMeshPool.Add(SplineMesh);
		}

		USplineMeshComponent* SplineMesh = TeleportPathMeshPool[i];
		SplineMesh->SetVisibility(true);

		FVector StartPos, StartTangent, EndPos, EndTangent;
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent);
		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);

	}
}

void APlayerCharacterVR::UpdateSpline(const TArray<FVector>& Path)
{
	TeleportPath->ClearSplinePoints(false);
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		FVector LocalPos = TeleportPath->GetComponentTransform().InverseTransformPosition(Path[i]); // From world to local (or otherway?)
		FSplinePoint Point(i, LocalPos, ESplinePointType::Curve);
		TeleportPath->AddPoint(Point);
	}

	TeleportPath->UpdateSpline();
}


void APlayerCharacterVR::ThumbstickPressedLeft(const FInputActionValue& Value)
{
	HandTryingToTeleport = EControllerHand::Left;
}

void APlayerCharacterVR::ThumbstickReleasedLeft(const FInputActionValue& Value)
{
	if (HandTryingToTeleport == EControllerHand::Left)
	{
		HandTryingToTeleport = EControllerHand::AnyHand;
		if (!bCanTeleport) return;
		Teleport();
	}
}

void APlayerCharacterVR::ThumbstickPressedRight(const FInputActionValue& Value)
{
	HandTryingToTeleport = EControllerHand::Right;
}

void APlayerCharacterVR::ThumbstickReleasedRight(const FInputActionValue& Value)
{
	if (HandTryingToTeleport == EControllerHand::Right)
	{
		HandTryingToTeleport = EControllerHand::AnyHand;
		if (!bCanTeleport) return;
		Teleport();
	}
}

void APlayerCharacterVR::Move(const FInputActionValue& Value)
{
	APlayerCharacter::Move(Value);
	if (MovementWay != EMovementWay::Teleport)
	{
		FVector2D MovementVector = Value.Get<FVector2D>();

		switch (WalkingOrientation)
		{
		case EWalkingOrientation::HMD:
		{
			FVector CameraForward = Orientation->GetForwardVector();
			FVector CameraRight = Orientation->GetRightVector();
			AddMovementInput(CameraForward, MovementVector.Y);
			AddMovementInput(CameraRight, MovementVector.X);
			break;
		}
		case EWalkingOrientation::LeftHand:
		{
			AddMovementInput(HandControllerLeft->GetActorForwardVector(), MovementVector.Y);
			AddMovementInput(HandControllerLeft->GetActorRightVector(), MovementVector.X);
			break;
		}
		case EWalkingOrientation::RightHand:
		{
			AddMovementInput(HandControllerRight->GetActorForwardVector(), MovementVector.Y);
			AddMovementInput(HandControllerRight->GetActorRightVector(), MovementVector.X);
			break;
		}
		}
	}
}

void APlayerCharacterVR::Rotate(const FInputActionValue& Value)
{
	float RotateAmount = Value.Get<float>();
	if (Controller)
	{
		int RotateDirection = RotateAmount > 0 ? 1 : -1;
		Controller->SetControlRotation(Controller->GetControlRotation() + FRotator(0, DegreesToRotate * RotateDirection, 0));
	}
}