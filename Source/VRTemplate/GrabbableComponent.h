// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrabbableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrabbed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReleased);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VRTEMPLATE_API UGrabbableComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	virtual void BeginPlay() override;

	UGrabbableComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	TArray<class AHandController*> GrabberHands;

	UPROPERTY(EditAnywhere)
	bool bOneHanded = true;

public:	
	
	virtual void Grab(class AHandController* Hand);
	virtual void Release(class AHandController* Hand);

	TArray <class AHandController*> GetGrabberHands() const { return GrabberHands; }
	bool IsOneHanded() const { return bOneHanded; }

	UPROPERTY(BlueprintAssignable)
	FOnGrabbed OnGrabbed;

	UPROPERTY(BlueprintAssignable)
	FOnReleased OnReleased;
};
