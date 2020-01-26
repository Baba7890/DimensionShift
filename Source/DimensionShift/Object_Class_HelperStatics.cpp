#include "Object_Class_HelperStatics.h"

UObject_Class_HelperStatics::UObject_Class_HelperStatics(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	
}

void UObject_Class_HelperStatics::EnableActor(AActor* ChosenActor, bool bIsEnabled)
{
	ChosenActor->SetActorHiddenInGame(!bIsEnabled);
	ChosenActor->SetActorEnableCollision(bIsEnabled);
	ChosenActor->SetActorTickEnabled(bIsEnabled);
}