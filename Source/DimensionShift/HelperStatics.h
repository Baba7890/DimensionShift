// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "HelperStatics.generated.h"

/**
 * 
 */
UCLASS()
class DIMENSIONSHIFT_API UHelperStatics : public UObject
{
	GENERATED_BODY()
	
public:
	UHelperStatics(const FObjectInitializer& ObjectInitializer);

	static void EnableActor(AActor* ChosenActor, bool bIsEnabled);
};
