//LevelBox is an Actor class that contains a trigger and a number of LevelObstacles.
//Used for the following:
//a) Adding itself to the GameInstance
//b) Uses the trigger to check whether the player is inside of it
//c) [Called from GameInstance] On dimension swapping to 2D, disables itself and its LevelObstacles when it does not contain the player

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
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
	 * This function enables/disables this level box along with its attached actors.
	 * Useful when the game is in 2D and we only want one of the level boxes to be active while the rest are deactivated
	 * @param - bIsIn2D -> Is the game in 2D or 3D? 2D = True, 3D = False
	*/
	void EnableLevelBox(bool bIsIn2D);
};
