//LevelObstacle is a class that includes any Actor that will shift a player or enemy's position when landed on.
//It contains four attached components: A StaticMesh and three BoxComponents, two colliders and one trigger.
//Used for the following:
//a) On 2D->3D dimension swapping, disables 2D collider and enables 3D collider
//b) [Called in LevelBox] Subscribes itself to the GameInstance OnDimensionSwapped delegate
//c) Moves player or enemy to its obstacle baseline if they enter trigger
//d) Makes itself become translucent when the player is behind it in 2D

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Level_Class_LevelObstacle.generated.h"

class ALevel_Class_LevelBox;
class UGameInstance_Class;
class APlayer_Class_MovementShift;

UCLASS()
class DIMENSIONSHIFT_API ALevel_Class_LevelObstacle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevel_Class_LevelObstacle();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		float obstacleBaselineYPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		UBoxComponent* ObstacleCollider2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		UBoxComponent* ObstacleCollider3D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		UBoxComponent* StandingOnTrigger;

	bool bIsPlayerInside = false;

private:
	ALevel_Class_LevelBox* ParentLevelBox;
	APlayer_Class_MovementShift* Player;
	UGameInstance_Class* GI;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Will be called by game instance OnDimensionSwapped delegate
	UFUNCTION()
	void DoSwapDimensionAction(bool bIsIn3D);

	/**
	 * This function adds a method to the game instance's OnDimensionSwapped delegate. Called from Level.cpp. I know, its messy.
	 * @param - bShouldAdd -> Should the method be subscribed to the delegate or removed?
	 */
	void SubscribeSwapMethodToGameInstance(bool bShouldAdd);

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnColliderEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	/**
	 * Checks and moves ChosenActor to this obstacle's baseline.
	 * + If the ChosenActor cannot move to the baseline, the ChosenActor is not moved, the obstacle allows the player to move inside of it and becomes translucent.
	 * @param - ChosenActor -> The AActor to be moved to this obstacle's baseline.
	 */
	void CheckAndMoveActorToBaseline(AActor* ChosenActor);
};