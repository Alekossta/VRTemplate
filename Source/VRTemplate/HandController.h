// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HandController.generated.h"

UCLASS()
class VRTEMPLATE_API AHandController : public AActor
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:

	// Functions

	AHandController();
	virtual void Tick(float DeltaTime) override;
	void SetTrackingSource(EControllerHand HandToSet);
	class UMotionControllerComponent* GetMotionControllerComp();
	USceneComponent* GetGripPoint() { return GripPoint; };
	void SetShowHand(bool bShow);
	float GetHandStrength() { return Strength; };

	void Grab();
	void Release();

	void Activate();
	void Deactivate();

	UFUNCTION(BlueprintCallable)
	FVector GetMotionVelocity() const;



private:

	// Components

	UPROPERTY(VisibleAnywhere)
	class UMotionControllerComponent* MotionController;

	UPROPERTY(VisibleAnywhere)
	class USkeletalMeshComponent* HandMesh;

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* GripPoint;

	// PROPERTIES

	UPROPERTY(EditDefaultsOnly)
	float Strength = 5.0f;

	UPROPERTY(EditDefaultsOnly)
	float MaxGrabDistance = 25.f;

	UPROPERTY(EditDefaultsOnly)
	float GrabOffset = 5.0f;

	UPROPERTY(EditDefaultsOnly)
	float GrabRadius = 3.0f;

	// Variables

	bool bIsGrabbing = false;
	class UGrabbableStaticMeshComponent* GrabbedComponent = nullptr;
	FVector PrevLocation;
	FVector CurrentLocation;
	FVector Velocity;

	// Functions

	void HandleVelocity();
};
