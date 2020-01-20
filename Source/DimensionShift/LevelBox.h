// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "LevelBox.generated.h"

UCLASS()
class DIMENSIONSHIFT_API ALevelBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelBox();

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

	UFUNCTION()
		void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void EnableLevelBox(bool bIsIn2D);
};
