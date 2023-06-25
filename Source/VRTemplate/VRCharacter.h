// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "HandController.h"
#include "InputAction.h"
#include "VRCharacter.generated.h"

UENUM()
enum class EHand
{
	Right,
	Left,
	None
};

UENUM()
enum class EMovementWay
{
	Teleport,
	Walking,
	Both
};

UENUM()
enum class EWalkingOrientation
{
	HMD,
	LeftHand,
	RightHand
};

UENUM()
enum class EPose
{
	Standing,
	Sitting
};

UCLASS()
class VRTEMPLATE_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	UPROPERTY()
	class AHandController* HandControllerRight;

	UPROPERTY()
	class AHandController* HandControllerLeft;

protected:
	virtual void BeginPlay() override;

	AVRCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:

	// COMPONENTS // 

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* VRCamera;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* VRRoot;

	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* Orientation;

	UPROPERTY(VisibleAnywhere)
	class USplineComponent* TeleportPath;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DestinationMarker;

	// FUNCTIONS //

	void SpawnHands();

	void KeepCameraAndCapsuleTogether();

	void ThumbstickPressedLeft(const FInputActionValue& Value);
	void ThumbstickPressedRight(const FInputActionValue& Value);
	void ThumbstickReleasedLeft(const FInputActionValue& Value);
	void ThumbstickReleasedRight(const FInputActionValue& Value);

	// Teleport
	void Teleport();
	void UpdateDestinationMarker();
	bool FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation);
	void UpdateSpline(const TArray<FVector>& Path);
	void DrawTeleportPath(const TArray<FVector>& Path);
	void FinishTeleport();

	// Walking
	void Move(const FInputActionValue& Value);
	void Rotate(const FInputActionValue& Value);

	// PROPERTIES // 

	UPROPERTY(EditDefaultsOnly, Category="Hands")
	TSubclassOf<class AHandController> HandControllerClassL;

	UPROPERTY(EditDefaultsOnly, Category = "Hands")
	TSubclassOf<class AHandController> HandControllerClassR;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	EPose Pose = EPose::Standing;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DegreesToRotate;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	EMovementWay MovementWay;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	EWalkingOrientation WalkingOrientation;

	EHand HandTryingToTeleport;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportProjectileRadius; // how big is the projectile we are simulating

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportProjectileSpeed;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportSimulationTime; // The time unreal lets the simulation of the projectile happen

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	FVector TeleportProjectionExtent = FVector(100, 100, 100); // The tolerance of the nav mesh on all axis I guess

	UPROPERTY()
	TArray<class USplineMeshComponent*> TeleportPathMeshPool;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	UStaticMesh* TeleportArchMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	class UMaterialInterface* TeleportArchMaterial;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* RotateAction;

	UPROPERTY (EditDefaultsOnly, Category = "Input")
	class UInputAction* TeleportLeftAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* TeleportRightAction;

	// VARIABLES //
	bool bCanTeleport;
};
