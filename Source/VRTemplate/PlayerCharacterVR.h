// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "InputAction.h"
#include "PlayerCharacterVR.generated.h"

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
class VRTEMPLATE_API APlayerCharacterVR : public APlayerCharacter
{
	GENERATED_BODY()

protected:

	// FUNCTIONS // 

	virtual void BeginPlay() override;

	APlayerCharacterVR();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void KeepCameraAndCapsuleTogether();

	virtual void SpawnHands() override;

	UFUNCTION()
	void ThumbstickPressedLeft(const FInputActionValue& Value);

	UFUNCTION()
	void ThumbstickPressedRight(const FInputActionValue& Value);

	UFUNCTION()
	void ThumbstickReleasedLeft(const FInputActionValue& Value);

	UFUNCTION()
	void ThumbstickReleasedRight(const FInputActionValue& Value);

	// Teleport
	void Teleport();
	void UpdateDestinationMarker();
	bool FindTeleportDestination(TArray<FVector>& OutPath, FVector& OutLocation);
	void UpdateSpline(const TArray<FVector>& Path);
	void DrawTeleportPath(const TArray<FVector>& Path);
	void FinishTeleport();

	UFUNCTION()
	void Rotate(const FInputActionValue& Value);

	virtual void Move(const FInputActionValue& Value) override;

	// COMPONENTS //

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* VRRoot;

	UPROPERTY(VisibleAnywhere)
	class UArrowComponent* Orientation;

	UPROPERTY(VisibleAnywhere)
	class USplineComponent* TeleportPath;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DestinationMarker;

	// PROPERTIES //

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	EPose Pose = EPose::Standing;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float DegreesToRotate;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	EMovementWay MovementWay;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	EWalkingOrientation WalkingOrientation;

	// how big is the projectile we are simulating
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportProjectileRadius; 

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportProjectileSpeed;

	// The time unreal lets the simulation of the projectile happen
	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	float TeleportSimulationTime;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	FVector TeleportProjectionExtent = FVector(10, 10, 10);

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	UStaticMesh* TeleportArchMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	class UMaterialInterface* TeleportArchMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* RotateAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* TeleportLeftAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* TeleportRightAction;


private:

	EControllerHand HandTryingToTeleport;

	UPROPERTY()
	TArray<class USplineMeshComponent*> TeleportPathMeshPool;

	bool bCanTeleport;

};
