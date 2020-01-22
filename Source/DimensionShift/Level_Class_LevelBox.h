// The level box implementation Tan wanted

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Level_Class_LevelBox.generated.h"

UCLASS()
class DIMENSIONSHIFT_API ALevel_Class_LevelBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevel_Class_LevelBox();

	UPROPERTY(EditAnywhere, Category = "Level Box")
		float baselineYPos;

	bool bIsPlayerInBox = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Level Box")
		UBoxComponent* BoxCollider;

private:
	TArray<AActor*> AttachedObjects;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//These two functions below serve as Unreal's OnTriggerEnter and OnTriggerExit respectively
	//They start being called after PostInitializeComponents() and before BeginPlay()

	UFUNCTION()
		void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
	* This function enables/disables this level box along with its attached actors.
	* Useful when the game is in 2D and we only want one of the level boxes to be active while the rest are deactivated
	* @param - bIsIn2D -> Is the game in 2D or 3D? 2D = True, 3D = False
	*/
	void EnableLevelBox(bool bIsIn2D);
};
