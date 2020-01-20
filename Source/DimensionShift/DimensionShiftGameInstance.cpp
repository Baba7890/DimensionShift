// Fill out your copyright notice in the Description page of Project Settings.

#include "DimensionShiftGameInstance.h"
#include "LevelBox.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

UDimensionShiftGameInstance::UDimensionShiftGameInstance()
{

}

void UDimensionShiftGameInstance::SwapDimensions()
{
	bIsIn3D = !bIsIn3D;

	int levelBoxWithPlayerNo = 999;

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
		OnDimensionSwapped.Broadcast(bIsIn3D, LevelBoxes[levelBoxWithPlayerNo]->baselineYPos);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("There are no level boxes that contain the player."));
	}
}

void UDimensionShiftGameInstance::AddLevelBox(ALevelBox* LevelBox)
{
	LevelBoxes.Add(LevelBox);

	if (LevelBox->bIsPlayerInBox)
	{
		FVector CharacterPos = GetWorld()->GetFirstPlayerController()->GetCharacter()->GetActorLocation();
		FVector NewCharacterPos = FVector(CharacterPos.X, LevelBox->baselineYPos, CharacterPos.Z);
		GetWorld()->GetFirstPlayerController()->GetCharacter()->SetActorLocation(NewCharacterPos);
	}
}

int32 UDimensionShiftGameInstance::GetNoOfLevelBoxes()
{
	return LevelBoxes.Num();
}