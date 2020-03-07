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
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
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
		float obstacleBaselineLocalYPos;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		UBoxComponent* ObstacleCollider2D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		UBoxComponent* ObstacleCollider3D;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		UBoxComponent* StandingOnTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		bool bDoesBaselineUseObstacleCenter = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dimension Obstacle")
		bool bDoesTriggerUseColliderScale = true;

private:
	ALevel_Class_LevelBox* ParentLevelBox;
	APlayer_Class_MovementShift* Player;
	UGameInstance_Class* GI;

	//Higher humber means higher priority on player y pos if the player is overlapping multiple level obstacles
	int priorityOverPlayerPosition = -1;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnColliderEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnColliderBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * Returns priorityOverPlayerPosition. Prevents setting of it
	 */
	int GetPriorityOverPlayerPosition();

	/**
	 * Returns the obstacle baseline y position depending on whether bDoesBaselineUseObstacleCenter is true or not
	 */
	float GetObstacleBaselinePosition();

	/**
	 * Sets the collision profile name and material opacity if the player is behind the obstacle in 2D
	 * @param - ProfileName -> The name of the collision profile to switch to
	 * @param - opacityValue -> The opacity amount between 0.0f and 1.0f
	 */
	void SetCollisionProfileNameAndOpacity(FName ProfileName, float opacityValue);
};
