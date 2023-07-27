// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "GrabbableStaticMeshComponent.generated.h"

class AHandController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabStart, AHandController*, GrabbingHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGrabEnd, AHandController*, GrabbingHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActivateStart, AHandController*, ActivatingHand);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActivateEnd, AHandController*, ActivatingHand);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class VRTEMPLATE_API UGrabbableStaticMeshComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	

protected:
	UGrabbableStaticMeshComponent();

	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	// FUNCTIONS

	virtual void GrabStart(class AHandController* Hand);
	virtual void GrabEnd(class AHandController* Hand);
	virtual void ActivateStart(class AHandController* Hand);
	virtual void ActivateEnd(class AHandController* Hand);
	bool IsOneHanded() { return bIsOneHanded; };
	TArray<class AHandController*> GetGrabberHands() { return GrabberHands; }

	UFUNCTION(BlueprintCallable)
	void ForceDrop();

	// PROPERTIES

	UPROPERTY(BlueprintAssignable)
	FOnGrabStart OnGrabStart;

	UPROPERTY(BlueprintAssignable)
	FOnGrabEnd OnGrabEnd;

	UPROPERTY(BlueprintAssignable)
	FOnActivateStart OnActivateStart;

	UPROPERTY(BlueprintAssignable)
	FOnActivateEnd OnActivateEnd;

private:

	// PROPERTIES

	UPROPERTY(EditAnywhere)
	bool bIsOneHanded = true;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bIsOneHanded"))
	bool bShouldAttach = false;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bShouldAttach"))
	bool bShouldSnap;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "bShouldAttach"))
	bool bShouldSimulatePhysics = false;

	// VARIABLES

	TArray<class AHandController* > GrabberHands;
};
