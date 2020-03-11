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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDimensionSwapDelegate, float, swapDuration);

UCLASS(BlueprintType, Blueprintable)
class DIMENSIONSHIFT_API APlayer_Class_MovementShift : public ACharacter
{
	GENERATED_BODY()

public:
	APlayer_Class_MovementShift(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "Dimension")
		FDimensionSwapDelegate OnDimensionSwapCallback;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension")
		bool bIsIn3D = false;

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

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<APlayer_Class_Weapon> WeaponActor;

	bool bHasGun = true;

	//TArray<ALevel_Class_LevelObstacle*> ObstacleTriggersInside;
	TArray<ALevel_Class_LevelObstacle*> ObstacleCollidersInside;

	#pragma region Health and Steam Stat Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		float currentHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		float maxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		float currentSteam = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		float maxSteam = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		float steamRegenAmount = 0.01f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		float steamRegenIntervalInSeconds = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
		float steamRegenDelayInSeconds = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
	bool bIsCurrentlyRegenSteam = false;
	#pragma endregion

	#pragma region Movement Variables

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Movement")
		float doubleJumpSteamUsage = 20.0f;

	bool bIsDashing = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Movement")
		float dashSteamUsage = 30.0f;

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
	 */
	UFUNCTION()
	void DoSwapDimensionAction();

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
