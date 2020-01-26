#include "GameInstance_Class.h"
#include "Level_Class_LevelBox.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

UGameInstance_Class::UGameInstance_Class()
{
	
}

void UGameInstance_Class::SwapDimensions()
{
	bIsIn3D = !bIsIn3D;

	bool hasFoundPlayerLevelBox = false;

	if (GetNoOfLevelBoxes() > 0)
	{
		for (int i = 0; i < GetNoOfLevelBoxes(); i++)
		{
			if (LevelBoxes[i]->bIsPlayerInBox && !hasFoundPlayerLevelBox)
				hasFoundPlayerLevelBox = true;
			else
				LevelBoxes[i]->EnableLevelBox(bIsIn3D);
		}
	}

	OnDimensionSwapped.Broadcast(bIsIn3D);
}

void UGameInstance_Class::AddLevelBox(ALevel_Class_LevelBox* LevelBox)
{
	LevelBoxes.Add(LevelBox);
}

int32 UGameInstance_Class::GetNoOfLevelBoxes()
{
	return LevelBoxes.Num();
}

float UGameInstance_Class::GetPlayerInLevelBoxBaseline()
{
	//This section of code is very vulnerable. What if there are multiple level boxes with bIsPlayerBox = true?
	for (int i = 0; i < GetNoOfLevelBoxes(); i++)
	{
		if (LevelBoxes[i]->bIsPlayerInBox)
			return LevelBoxes[i]->baselineYPos;
	}

	UE_LOG(LogTemp, Error, TEXT("There are no level boxes that contain the player."));
	return 0.0f;
}

void UGameInstance_Class::ClearCurrentLevelBoxes()
{
	LevelBoxes.Empty();
	bIsIn3D = false;
}
