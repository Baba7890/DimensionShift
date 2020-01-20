// Fill out your copyright notice in the Description page of Project Settings.


#include "HelperStatics.h"

UHelperStatics::UHelperStatics(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UHelperStatics::EnableActor(AActor* ChosenActor, bool bIsEnabled)
{
	ChosenActor->SetActorHiddenInGame(!bIsEnabled);
	ChosenActor->SetActorEnableCollision(bIsEnabled);
	ChosenActor->SetActorTickEnabled(bIsEnabled);
}