//LevelBox is an Actor class that contains a trigger and a number of LevelObstacles.
//Used for the following:
//a) Adding itself to the GameInstance
//b) Uses the trigger to check whether the player is inside of it
//c) [Called from GameInstance] On dimension swapping to 2D, disables itself and its LevelObstacles when it does not contain the player

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "TimerManager.h"
#include "Level_Class_LevelBox.generated.h"

class UGameInstance_Class;

UCLASS()
class DIMENSIONSHIFT_API ALevel_Class_LevelBox : public AActor
{
	GENERATED_BODY()
	
public:	
	ALevel_Class_LevelBox();

	UPROPERTY(EditAnywhere, Category = "Level Box")
	float baselineYPos;

	bool bIsPlayerInBox = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Box")
	UBoxComponent* BoxCollider;

private:
	TArray<AActor*> AttachedObjects;
	UGameInstance_Class* GI;

	FTimerHandle DimensionTimerHandle;
	FTimerDelegate DimensionTimerDelegate;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
	 * This method is played when the world switches from 2D -> 3D or vice versa
	 * + This method calls EnableLevelBox() after a certain amount of time.
	 * + Unlike many other Actors, LevelBox's DoDimensionSwapAction is NOT connected to the GameInstance's delegate
	 * @param - bPlayerIsIn3D -> Is the player in 3D? TRUE = 3D, FALSE = 2D
	 * @param - swapDuration -> The duration it takes to swap between 2D -> 3D or vice versa.
	 */
	void DoDimensionSwapAction(bool bPlayerIsIn3D, float swapDuration);

	/**
	 * This function enables/disables this level box along with its attached actors.
	 * Useful when the game is in 2D and we only want one of the level boxes to be active while the rest are deactivated
	 * @param - bPlayerIsIn3D -> Is the player in 3D? If so, enable this LevelBox. If not, disable it...unless the player is in it
	 */
	UFUNCTION()
	void EnableLevelBox(bool bPlayerIsIn3D);
};
