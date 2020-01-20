// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "MainPlayer.generated.h"

UCLASS()
class DIMENSIONSHIFT_API AMainPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMainPlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	float twoDimensionYaw = -90.0f;

private:
	bool bIsUsing3DControls = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Dimension Swap")
	void UseSwapDimensionAbility();

	UFUNCTION()		//This property should be placed when this function will be called by delegate
	void DoSwapDimensionAction(bool bIsIn3D, float baselineYPos);

private:
	UFUNCTION(BlueprintCallable, Category = "Player Movement")
	void MoveForward(float fAxis);

	UFUNCTION(BlueprintCallable, Category = "Player Movement")
	void MoveRight(float fAxis);

	void TurnTo3D();
	void TurnTo2D(float baselineYPos);
};
