#include "GameInstance_Class.h"
#include "Level_Class_LevelBox.h"		//We can use the .h file here. No forward declaration will happen
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

UGameInstance_Class::UGameInstance_Class()
{
	//Nothing here
}

void UGameInstance_Class::SwapDimensions()
{
	//Swaps from 2D to 3D or vice versa
	bIsIn3D = !bIsIn3D;

	int levelBoxWithPlayerNo = 999;

	//This part controls which level box will be disabled/enabled.
	//The one that the player is in will NOT be disabled when swapping to 2D
	if (GetNoOfLevelBoxes() > 0)
	{
		for (int i = 0; i < GetNoOfLevelBoxes(); i++)
		{
			if (!LevelBoxes[i]->bIsPlayerInBox)
			{
				LevelBoxes[i]->EnableLevelBox(bIsIn3D);
			}
			else
			{
				levelBoxWithPlayerNo = i;
			}
		}
	}

	if (levelBoxWithPlayerNo < GetNoOfLevelBoxes())
	{
		//Delegate broadcast. Similar to Unity delegate's Invoke()
		OnDimensionSwapped.Broadcast(bIsIn3D, LevelBoxes[levelBoxWithPlayerNo]->baselineYPos);
	}
	else
	{
		//How you show errors in the Output Log window in Unreal Editor
		UE_LOG(LogTemp, Error, TEXT("There are no level boxes that contain the player."));
	}
}

void UGameInstance_Class::AddLevelBox(ALevel_Class_LevelBox* LevelBox)
{
	LevelBoxes.Add(LevelBox);

	//If this LevelBox has a player initially already, we set the player to the 2D baseline of this Level Box
	if (LevelBox->bIsPlayerInBox)
	{
		FVector CharacterPos = GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorLocation();
		FVector NewCharacterPos = FVector(CharacterPos.X, LevelBox->baselineYPos, CharacterPos.Z);
		GetWorld()->GetFirstPlayerController()->GetCharacter()->SetActorLocation(NewCharacterPos);
	}
}

int32 UGameInstance_Class::GetNoOfLevelBoxes()
{
	return LevelBoxes.Num();
}
