// This will be the game's game instance. It serves as a singleton/manager. There can only be one of these during the entire game.
// We define this is the Game Instance at the bottom of the "Maps and Modes" under Unreal Editor's "Project Settings".

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameInstance_Class.generated.h"

class ALevel_Class_LevelBox;

UCLASS()
class DIMENSIONSHIFT_API UGameInstance_Class : public UGameInstance
{
	GENERATED_BODY()

public:
	float swapDuration = 2.0f;

private:
	//This is how we will declare Arrays
	TArray<ALevel_Class_LevelBox*> LevelBoxes;

public:
	UGameInstance_Class();
	
	/**
	 * This function is called by the player whenever the player is going to swap dimensions. Important function.
	 */
	void EnableLevelBoxes(bool bPlayerIsIn3D);

	/**
	 * This function adds a LevelBox to this GameInstance;s LevelBoxes TArray.
	 * @param - LevelBox -> The LevelBox to add
	 */
	void AddLevelBox(ALevel_Class_LevelBox* LevelBox);

	int GetNoOfLevelBoxes();

	/**
	 * This function gets and returns the Y baseline location of the LevelBox the player is in.
	 */
	float GetPlayerInLevelBoxBaseline();

	/**
	 * This function clears all of the level boxes from the LevelBoxes TArray
	 */
	UFUNCTION(BlueprintCallable, Category = "Game Instance")
	void ClearCurrentLevelBoxes();
};
