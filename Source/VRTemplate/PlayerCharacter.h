#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputAction.h"
#include "PlayerCharacter.generated.h"

UCLASS()
class VRTEMPLATE_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

protected:

	// COMPONENTS //

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	// FUNCTIONS //

	APlayerCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void Move(const FInputActionValue& Value);

	virtual void SpawnHands();

	UFUNCTION()
	void GrabLeft();

	UFUNCTION()
	void GrabRight();

	UFUNCTION()
	void ReleaseLeft();

	UFUNCTION()
	void ReleaseRight();

	UFUNCTION()
	void ActivateRight();

	UFUNCTION()
	void DeactivateRight();

	UFUNCTION()
	void ActivateLeft();

	UFUNCTION()
	void DeactivateLeft();


	// PROPERTIES // 

	UPROPERTY(EditDefaultsOnly, Category = "Hands")
	TSubclassOf<class AHandController> HandControllerClassL;

	UPROPERTY(EditDefaultsOnly, Category = "Hands")
	TSubclassOf<class AHandController> HandControllerClassR;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* GripRightAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* GripLeftAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ActivateRightAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* ActivateLeftAction;

	// VARIABLES //

	UPROPERTY()
	class AHandController* HandControllerRight;

	UPROPERTY()
	class AHandController* HandControllerLeft;

	UPROPERTY()
	class USceneComponent* HandsOwner;

};
