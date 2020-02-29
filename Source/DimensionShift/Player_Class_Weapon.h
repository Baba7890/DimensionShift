// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Player_Class_Weapon.generated.h"

class AProjectile_Class_ProjBase;
class APlayer_Class_MovementShift;
class UGameInstance_Class;

UCLASS()
class DIMENSIONSHIFT_API APlayer_Class_Weapon : public AActor
{
	GENERATED_BODY()
	
public:	
	APlayer_Class_Weapon();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		USceneComponent* ProjectileSpawner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		USphereComponent* Trigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
		UProjectileMovementComponent* WeaponProjectileComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<AProjectile_Class_ProjBase> Projectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float fireDelay = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float shootSteamUsage = 2.0f;

	bool bIsShooting = false;

	#pragma region Throw Weapon variables

	bool bIsThrowing = false;
	bool bIsCharging = false;
	bool bIsGoingForward = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float throwChargeSpeedInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float currentThrowCharge = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float throwChargeAmountByInterval = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float throwMaxDistance = 3000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
		float throwStunDuration = 1.0f;

	#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerOwner")
		APlayer_Class_MovementShift* PlayerOwner;	

private:
	FTimerHandle FireDelayTimerHandle;
	FTimerHandle ThrowChargeTimerHandle;
	FTimerHandle ReturnToPlayerTimerHandle;
	UGameInstance_Class* GI;

	FVector OldPosition;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	/**
	 * Spawns Projectile from the ProjectileSpawner of this weapon.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shooting")
	void FireWeapon();

	/**
	 * Resets bIsShooting to true
	 */
	void ResetIsShooting();

	/**
	 * Prepares to throw the weapon by resetting values. The actual throw charging is done in Tick()
	 */
	UFUNCTION(BlueprintCallable, Category = "Shooting")
	void PrepareToThrowWeapon();

	/**
	 * Throws itself forward.
	 */
	UFUNCTION(BlueprintCallable, Category = "Shooting")
	void ThrowWeapon();

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	/**
	 * Increments the potential throw distance of this weapon until it reaches maxThrowSpeed
	 */
	void IncrementThrowCharge();

	/**
	 * Returns the weapon back to the player
	 * LOC - Called when the weapon has reached its max distance, OR when the weapon has hit something other than the player
	 */
	void ReturnToPlayer();
};
