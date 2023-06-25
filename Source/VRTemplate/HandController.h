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
	void SetShowHand(bool bShow);

private:

	// Components

	UPROPERTY(VisibleAnywhere)
	class UMotionControllerComponent* MotionController;

	UPROPERTY(VisibleAnywhere)
	class USkeletalMeshComponent* HandMesh;
};
