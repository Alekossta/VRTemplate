#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/ShapeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ArrowComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

AVRCharacter::AVRCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	VRCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	VRCamera->SetupAttachment(VRRoot);

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

void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);
	HandTryingToTeleport = EHand::None;

	// Hide all teleport path spline meshes
	for (USplineMeshComponent* SplineMesh : TeleportPathMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}

	SpawnHands();

	// Setup Enhanced Input contexts
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
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

void AVRCharacter::SpawnHands()
{
	HandControllerLeft = GetWorld()->SpawnActor<AHandController>(HandControllerClassL);
	if (HandControllerLeft != nullptr)
	{
		HandControllerLeft->SetTrackingSource(EControllerHand::Left);
		HandControllerLeft->SetOwner(this);
		HandControllerLeft->AttachToComponent(VRRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	HandControllerRight = GetWorld()->SpawnActor<AHandController>(HandControllerClassR);
	if (HandControllerRight != nullptr)
	{
		HandControllerRight->SetTrackingSource(EControllerHand::Right);
		HandControllerRight->SetOwner(this);
		HandControllerRight->AttachToComponent(VRRoot, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVRCharacter::Move);
		EnhancedInputComponent->BindAction(RotateAction, ETriggerEvent::Started, this, &AVRCharacter::Rotate);
		EnhancedInputComponent->BindAction(TeleportLeftAction, ETriggerEvent::Started, this, &AVRCharacter::ThumbstickPressedLeft);
		EnhancedInputComponent->BindAction(TeleportLeftAction, ETriggerEvent::Completed, this, &AVRCharacter::ThumbstickReleasedLeft);
		EnhancedInputComponent->BindAction(TeleportRightAction, ETriggerEvent::Started, this, &AVRCharacter::ThumbstickPressedRight);
		EnhancedInputComponent->BindAction(TeleportRightAction, ETriggerEvent::Completed, this, &AVRCharacter::ThumbstickReleasedRight);
	}
}

void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	KeepCameraAndCapsuleTogether();
	UpdateDestinationMarker();
}

void AVRCharacter::KeepCameraAndCapsuleTogether()
{
	if (VRCamera == nullptr || GetArrowComponent() == nullptr) return; // Do some pointer checks

	// Make arrow of character to follow camera z axis rotation so that we have an orientation reference
	float RotationYaw = VRCamera->GetComponentRotation().Yaw;
	FRotator Rotation = FRotator(Orientation->GetComponentRotation().Pitch, RotationYaw,
		Orientation->GetComponentRotation().Roll);
	Orientation->SetWorldRotation(Rotation);

	FVector CameraOffsetPerFrame = VRCamera->GetComponentLocation() - GetActorLocation();
	CameraOffsetPerFrame.Z = 0; // Dont align z cause the capsule would move up and down with the headset
	AddActorWorldOffset(CameraOffsetPerFrame, true);
	if (VRRoot == nullptr) return;
	VRRoot->AddWorldOffset(-CameraOffsetPerFrame, true);
}

void AVRCharacter::FinishTeleport()
{
	DestinationMarker->SetVisibility(false);
	TArray<FVector> EmptyPath;

	DrawTeleportPath(EmptyPath);
	bCanTeleport = false;

	HandControllerLeft->SetShowHand(true);
	HandControllerRight->SetShowHand(true);
}


void AVRCharacter::Teleport()
{
	FVector Destination = DestinationMarker->GetComponentLocation();
	Destination += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();
	FHitResult* Hit = nullptr;
	SetActorLocation(Destination, false, Hit, ETeleportType::ResetPhysics);

	FinishTeleport();
}

void AVRCharacter::UpdateDestinationMarker()
{
	// dont't update the destination marker if not needed
	if (MovementWay == EMovementWay::Walking || HandTryingToTeleport == EHand::None) return; 

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

bool AVRCharacter::FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation)
{
	FVector Start;
	FVector Direction;
	FVector End;
	switch (HandTryingToTeleport)
	{
		case EHand::Left:
		{
			Start = HandControllerLeft->GetActorLocation();
			End = HandControllerLeft->GetActorLocation();
			Direction = HandControllerLeft->GetActorForwardVector();
			HandControllerLeft->SetShowHand(false);
			HandControllerRight->SetShowHand(true);
			break;
		}
		case EHand::Right:
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

void AVRCharacter::DrawTeleportPath(const TArray<FVector>& Path)
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

void AVRCharacter::UpdateSpline(const TArray<FVector>& Path)
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


void AVRCharacter::ThumbstickPressedLeft(const FInputActionValue& Value)
{
	HandTryingToTeleport = EHand::Left;
}

void AVRCharacter::ThumbstickReleasedLeft(const FInputActionValue& Value)
{
	if (HandTryingToTeleport == EHand::Left)
	{
		HandTryingToTeleport = EHand::None;
		if (!bCanTeleport) return;
		Teleport();
	}
}

void AVRCharacter::ThumbstickPressedRight(const FInputActionValue& Value)
{
	HandTryingToTeleport = EHand::Right;
}

void AVRCharacter::ThumbstickReleasedRight(const FInputActionValue& Value)
{
	if (HandTryingToTeleport == EHand::Right)
	{
		HandTryingToTeleport = EHand::None;
		if (!bCanTeleport) return;
		Teleport();
	}
}

void AVRCharacter::Move(const FInputActionValue& Value)
{
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

void AVRCharacter::Rotate(const FInputActionValue& Value)
{
	float RotateAmount = Value.Get<float>();
	if (Controller)
	{
		int RotateDirection = RotateAmount > 0 ? 1 : -1;
		Controller->SetControlRotation(Controller->GetControlRotation() + FRotator(0, DegreesToRotate * RotateDirection, 0));
	}
}