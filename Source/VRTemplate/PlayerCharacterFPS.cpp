#include "PlayerCharacterFPS.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "HandController.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"

APlayerCharacterFPS::APlayerCharacterFPS()
{
	Camera->bUsePawnControlRotation = true;
	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	Camera->SetRelativeLocation(FVector(0, 0, CapsuleHalfHeight));
}

void APlayerCharacterFPS::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacterFPS::Look);
	}
}

void APlayerCharacterFPS::SpawnHands()
{
	HandsOwner = Cast<USceneComponent>(Camera);
	APlayerCharacter::SpawnHands();

	HandControllerLeft->AddActorLocalOffset(FVector(50, -25, -10));
	HandControllerRight->AddActorLocalOffset(FVector(50, 25, -10));
}

void APlayerCharacterFPS::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	if (MovementVector.Size() > 0)
	{
		float XValue = MovementVector.X;
		float YValue = MovementVector.Y;
		FVector ForwardVector = GetActorForwardVector();
		FVector RightVector = GetActorRightVector();
		AddMovementInput(ForwardVector, YValue);
		AddMovementInput(RightVector, XValue);
	}
}

void APlayerCharacterFPS::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	if (LookVector.Size() > 0)
	{
		float XValue = LookVector.X;
		float YValue = LookVector.Y;
		AddControllerYawInput(XValue);
		AddControllerPitchInput(YValue);
	}
}