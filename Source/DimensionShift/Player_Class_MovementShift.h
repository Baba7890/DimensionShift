// Script of main character

#pragma once

//Include whatever you might need to use. Unreal doesn't include these things initially to quicken compile times.
//MAKE SURE WHATEVER YOU ADD IS BETWEEN THESE TWO INCLUDES
//#include "CoreMinimal.h"
//[Place extra includes here]
//#include "Player_Class_MovementShift.generated.h"

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "Player_Class_MovementShift.generated.h"

UCLASS()
class DIMENSIONSHIFT_API APlayer_Class_MovementShift : public ACharacter
{
	GENERATED_BODY()

public:
	APlayer_Class_MovementShift();

	//UPROPERTYs allow you to show or/and allow editing the variable in the editor.
	//VisibleAnywhere -> Can be seen in Editor but cannot be edited
	//EditAnywhere -> Can be seen and edited. Should only be used for normal data type variables, not objects
	//BlueprintReadOnly -> This object/variable/component can be gotten in blueprints
	//Category = "String" -> This allows you to categorize where the object/variable/component

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* CameraBoom;	//All references to objects of a class should be a pointer '*'.

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

	//UPROPERTYs allow you to let designer call functions from blueprints or allow a function to be subscribed to a delegate
	//BlueprintCallable -> This function can be called from the blueprints
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
