// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "TimerManager.h"
#include "Projectile_Class_ProjBase.generated.h"

class APlayer_Class_MovementShift;

UCLASS()
class DIMENSIONSHIFT_API AProjectile_Class_ProjBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile_Class_ProjBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
		USphereComponent* SphereCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
		UProjectileMovementComponent* ProjectileMovement;

private:
	APlayer_Class_MovementShift* Player;
	
	float currentLifeSpan = 0.0f;
	FVector preDimensionSwapVelocity;

	FTimerHandle ProjStopTimerHandle;
	FTimerHandle ProjLifeSpanTimerHandle;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UFUNCTION()
	void OnDimensionSwap(float swapDuration);

	void OnDimensionSwapEnd();
	void DestroyProjectile();
};
