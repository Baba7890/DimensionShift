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
class ALevel_Class_LevelObstacle;
class UPlayer_Class_CustomMoveComponent;

UCLASS()
class DIMENSIONSHIFT_API APlayer_Class_MovementShift : public ACharacter
{
	GENERATED_BODY()

public:
	APlayer_Class_MovementShift(const FObjectInitializer& ObjectInitializer);

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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
		float swapDuration = 2.0f;
	#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Movement")
		bool bCanPlayerMove = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Movement")
		bool bIsPlayerLookingRightIn2D = true;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<APlayer_Class_Weapon> WeaponActor;

	bool bHasGun = true;

	TArray<ALevel_Class_LevelObstacle*> LevelObstaclesInside;

	#pragma region Health and Steam Stat Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		int currentHealth = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		int maxHealth = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		int currentSteam = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		int maxSteam = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		int steamRegenAmount = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		float steamRegenDelayInSeconds = 1.0f;
	#pragma endregion

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Movement")
		int doubleJumpSteamUsage = 20;

	#pragma region Dash Variables

	bool bIsDashing = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Movement")
		int dashSteamUsage = 30;

	#pragma endregion

private:
	UGameInstance_Class* GI;
	UPlayer_Class_CustomMoveComponent* CustomMoveComponent;
	FTimerHandle DimensionTimerHandle;

	#pragma region Camera Variables

	float currentLerpAlpha = 0.0f;
	bool bHasFinishedViewLerp = false;

	#pragma endregion

	APlayer_Class_Weapon* Weapon;

	//The velocity to save when the player switches dimensions so we can apply the same velocity on the player post-dimension swap
	FVector PreDimensionSwapVelocity;

	#pragma region Health and Steam Stat Variables

	FTimerHandle SteamTimerHandle;
	bool bCanRegenerateSteam = true;

	#pragma endregion

	FTimerHandle DashTimerHandle;

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

	/**
	 * Inherited method that makes player jump.
	 * ADD - Player can only jump if the player is not dashing
	 */
	virtual void Jump() override;

	/**
	 * Reduces the currentSteam by amount. If it goes below 0, will be set to 0
	 * @param - amount -> The amount of steam to subtract from currentSteam
	 */
	UFUNCTION(BlueprintCallable, Category = "Steam Related")
	void ReduceSteam(int amount);
	
	/**
	 * Returns whether the player is currently looking right or left in 2D
	 */
	UFUNCTION(BlueprintCallable, Category = "Player Movement")
	bool GetIsPlayerLookingRightInTwoDimen();

private:
	/**
	 * Moves the player forward, backwards in 3D
	 * LOC - Called in the player blueprint.
	 * @param - fAxis -> The movement input value, between 0.0f and 1.0f
	 */
	UFUNCTION(BlueprintCallable, Category = "Player Movement")
	void MoveForward(float fAxis);

	/**
	 * Moves the player right, left in 3D AND 2D
	 * LOC - Called in the player blueprint.
	 * @param - fAxis -> The movement input value, between 0.0f and 1.0f
	 */
	UFUNCTION(BlueprintCallable, Category = "Player Movement")
	void MoveRight(float fAxis);

	/**
	 * Performs the dash action depending on whether the player is pressing any input or not.
	 * LOC - Called in the player blueprint.
	 */
	UFUNCTION(BlueprintCallable, Category = "Player Movement")
	void Dash();

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

	/**
	 * Regenerates currentSteam by steamRegenAmount. If it goes above maxSteam, set currentSteam to maxSteam.
	 * LOC - Called by a timer in this Actor's Tick() using SteamTimerHandle after steamRegenRateInSeconds
	 */
	void RegenSteam();

	/**
	 * Resets bIsDashing and stops player movement immediately
	 * LOC - Called by a timer in this Actor's Dash() using DashTimerHandle after dashDuration
	 */
	void ResetIsDashing();
};
