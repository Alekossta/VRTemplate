#include "PlayerCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HandController.h"


APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	Camera->SetupAttachment(GetRootComponent());
	Camera->FieldOfView = 110.f;
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
	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{

}