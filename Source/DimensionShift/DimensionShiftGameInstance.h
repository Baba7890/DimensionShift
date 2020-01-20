// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DimensionShiftGameInstance.generated.h"

class ALevelBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDimensionSwapped, bool, bIsIn3D, float, baselineYPos);

UCLASS()
class DIMENSIONSHIFT_API UDimensionShiftGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	bool bIsIn3D = false;
	FOnDimensionSwapped OnDimensionSwapped;

private:
	TArray<ALevelBox*> LevelBoxes;

public:
	UDimensionShiftGameInstance();
	void SwapDimensions();
	void AddLevelBox(ALevelBox* LevelBox);
	int GetNoOfLevelBoxes();
	
};
