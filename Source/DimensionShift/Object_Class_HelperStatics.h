//This class simply serves as a class that contains static helper functions

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "Object_Class_HelperStatics.generated.h"

UCLASS()
class DIMENSIONSHIFT_API UObject_Class_HelperStatics : public UObject
{
	GENERATED_BODY()
	
public:
	UObject_Class_HelperStatics(const FObjectInitializer& ObjectInitializer);

	/**
	 * This static function can be used to enable/disable an actor.
	 * @param - ChosenActor -> The Actor to be enabled/disabled
	 * @param - bIsEnabled -> Is this Actor going to be enabled or disabled? true = Enabled, false = Disabled
	 */
	static void EnableActor(AActor* ChosenActor, bool bIsEnabled);
};
