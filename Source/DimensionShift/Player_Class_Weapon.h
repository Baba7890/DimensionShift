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

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
		TSubclassOf<AProjectile_Class_ProjBase> Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
		float fireDelay = 1.0f;

	bool bIsShooting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerOwner")
		APlayer_Class_MovementShift* PlayerOwner;	

private:
	FTimerHandle FireDelayTimerHandle;
	UGameInstance_Class* GI;

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
};
