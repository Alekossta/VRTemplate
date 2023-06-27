#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterFPS.generated.h"

UCLASS()
class VRTEMPLATE_API APlayerCharacterFPS : public APlayerCharacter
{
	GENERATED_BODY()

protected:

	APlayerCharacterFPS();

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* LookAction;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Move(const FInputActionValue& Value) override;
	virtual void SpawnHands() override;

	void Look(const FInputActionValue& Value);
};
