#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HandController.h"
#include "Components/CapsuleComponent.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	Camera->SetupAttachment(GetRootComponent());
	Camera->FieldOfView = 110.f;

	GetCapsuleComponent()->SetCapsuleRadius(7.f);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnHands();

	// Setup Enhanced Input contexts
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

void APlayerCharacter::SpawnHands()
{
	if (HandsOwner == nullptr) return;

	HandControllerLeft = GetWorld()->SpawnActor<AHandController>(HandControllerClassL);
	if (HandControllerLeft != nullptr)
	{
		HandControllerLeft->SetTrackingSource(EControllerHand::Left);
		HandControllerLeft->SetOwner(this);
		HandControllerLeft->AttachToComponent(HandsOwner, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	HandControllerRight = GetWorld()->SpawnActor<AHandController>(HandControllerClassR);
	if (HandControllerRight != nullptr)
	{
		HandControllerRight->SetTrackingSource(EControllerHand::Right);
		HandControllerRight->SetOwner(this);
		HandControllerRight->AttachToComponent(HandsOwner, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EnhancedInputComponent->BindAction(GripRightAction, ETriggerEvent::Started, this, &APlayerCharacter::GrabRight);
		EnhancedInputComponent->BindAction(GripRightAction, ETriggerEvent::Completed, this, &APlayerCharacter::ReleaseRight);
		EnhancedInputComponent->BindAction(GripLeftAction, ETriggerEvent::Started, this, &APlayerCharacter::GrabLeft);
		EnhancedInputComponent->BindAction(GripLeftAction, ETriggerEvent::Completed, this, &APlayerCharacter::ReleaseLeft);
		
		EnhancedInputComponent->BindAction(ActivateRightAction, ETriggerEvent::Started, this, &APlayerCharacter::ActivateRight);
		EnhancedInputComponent->BindAction(ActivateRightAction, ETriggerEvent::Completed, this, &APlayerCharacter::DeactivateRight);
		EnhancedInputComponent->BindAction(ActivateLeftAction, ETriggerEvent::Started, this, &APlayerCharacter::ActivateLeft);
		EnhancedInputComponent->BindAction(ActivateLeftAction, ETriggerEvent::Completed, this, &APlayerCharacter::DeactivateLeft);
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{

}

void APlayerCharacter::GrabLeft()
{
	if (HandControllerLeft != nullptr)
	{
		HandControllerLeft->Grab();
	}
}

void APlayerCharacter::GrabRight()
{
	if (HandControllerRight != nullptr)
	{
		HandControllerRight->Grab();
	}
}

void APlayerCharacter::ReleaseLeft()
{
	if (HandControllerLeft != nullptr)
	{
		HandControllerLeft->Release();
	}
}

void APlayerCharacter::ReleaseRight()
{
	if (HandControllerRight != nullptr)
	{
		HandControllerRight->Release();
	}
}

void APlayerCharacter::ActivateRight()
{
	if (HandControllerRight)
	{
		HandControllerRight->Activate();
	}
}

void APlayerCharacter::DeactivateRight()
{
	if (HandControllerRight)
	{
		HandControllerRight->Deactivate();
	}
}

void APlayerCharacter::ActivateLeft()
{
	if (HandControllerLeft)
	{
		HandControllerLeft->Activate();
	}
}

void APlayerCharacter::DeactivateLeft()
{
	if (HandControllerLeft)
	{
		HandControllerLeft->Deactivate();
	}
}