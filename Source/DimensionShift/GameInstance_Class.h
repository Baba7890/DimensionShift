// This will be the game's game instance. It serves as a singleton/manager. There can only be one of these during the entire game.
// We define this is the Game Instance at the bottom of the "Maps and Modes" under Unreal Editor's "Project Settings".

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameInstance_Class.generated.h"

//This below a forward declaration. It prevents circular dependency. Look for "Unreal Forward Declartion" on Google.
class ALevel_Class_LevelBox;

//This below is a delegate declartion. Don't mind the errors, Unreal has a lot of false errors. .A lot of true ones too...
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDimensionSwap, bool, bIsIn3D, float, baselineYPos);

UCLASS()
class DIMENSIONSHIFT_API UGameInstance_Class : public UGameInstance
{
	GENERATED_BODY()

public:
	bool bIsIn3D = false;
	FOnDimensionSwap OnDimensionSwapped;

private:
	//This is how we will declare Arrays
	TArray<ALevel_Class_LevelBox*> LevelBoxes;

public:
	UGameInstance_Class();
	void SwapDimensions();
	void AddLevelBox(ALevel_Class_LevelBox* LevelBox);
	int GetNoOfLevelBoxes();
};
