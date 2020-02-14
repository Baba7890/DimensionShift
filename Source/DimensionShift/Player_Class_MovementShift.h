//This class includes player movement + dimension shifting (for the player) + transition camera movement effect

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Player_Class_MovementShift.generated.h"

class UGameInstance_Class;
class APlayer_Class_Weapon;

UCLASS()
class DIMENSIONSHIFT_API APlayer_Class_MovementShift : public ACharacter
{
	GENERATED_BODY()

public:
	APlayer_Class_MovementShift();

	#pragma region Camera Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* CameraBoom2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* FollowCamera2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* CameraBoom3D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* FollowCamera3D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		USpringArmComponent* TransCameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		UCameraComponent* TransitionCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float transCamTargetArmLength3D = 275.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float transCamTargetArmLength2D = 350000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float transCamFieldOfView3D = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float transCamFieldOfView2D = 0.3f;
	#pragma endregion

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float swapDuration = 2.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Movement")
		bool bCanPlayerMove = true;

	int noOfOverlappingObstacleTrigs = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<APlayer_Class_Weapon> WeaponActor;

	bool bHasGun = true;

private:
	UGameInstance_Class* GI;
	FTimerHandle DimensionTimerHandle;

	#pragma region Camera Variables

	float currentLerpAlpha = 0.0f;
	bool bHasFinishedViewLerp = false;

	#pragma endregion

	APlayer_Class_Weapon* Weapon;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/**
	 * Called when the player uses his dimension swap ability.
	 */
	UFUNCTION(BlueprintCallable, Category = "Dimension Swap")
	void UseSwapDimensionAbility();

	/**
	 * This method is played when the world switches from 2D -> 3D or vice versa
	 * + This method contains a timer that calls TurnTo2D() and TurnTo3D() after a set amount of time
	 * LOC - This method will be called the Game Instance's OnDimensionSwapped delegate
	 * @param - bIsIn3D -> Is the world in 3D? TRUE = 3D, FALSE = 2D
	 * @param - swapDura -> The duration it takes to swap between 2D -> 3D or vice versa.
	 */
	UFUNCTION()
	void DoSwapDimensionAction(bool bIsIn3D, float swapDura);

private:
	/**
	 * Moves the player forward, backwards in 3D
	 * @param - fAxis -> The movement input value, between 0.0f and 1.0f
	 */
	UFUNCTION(BlueprintCallable, Category = "Player Movement")
	void MoveForward(float fAxis);

	/**
	 * Moves the player right, left in 3D AND 2D
	 * @param - fAxis -> The movement input value, between 0.0f and 1.0f
	 */
	UFUNCTION(BlueprintCallable, Category = "Player Movement")
	void MoveRight(float fAxis);

	/**
	 * Called when the world swaps to 3D.
	 */
	void TurnTo3D();

	/**
	 * Called when the world swaps to 2D.
	 */
	void TurnTo2D();

	/**
	 * Performs the transition camera movement from 2D -> 3D and vice versa
	 * @param - deltaTime -> The time taken for the game to go from the previous frame to the current frame 
	 */
	void PerformTransitionCameraMovement(float deltaTime);
};
