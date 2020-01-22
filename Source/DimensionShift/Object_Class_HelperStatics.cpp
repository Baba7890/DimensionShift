#include "Object_Class_HelperStatics.h"

UObject_Class_HelperStatics::UObject_Class_HelperStatics(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//Nothing here.
}

void UObject_Class_HelperStatics::EnableActor(AActor* ChosenActor, bool bIsEnabled)
{
	//These three methods of Actor effectively enables/disables an actor completely.
	ChosenActor->SetActorHiddenInGame(!bIsEnabled);
	ChosenActor->SetActorEnableCollision(bIsEnabled);
	ChosenActor->SetActorTickEnabled(bIsEnabled);
}