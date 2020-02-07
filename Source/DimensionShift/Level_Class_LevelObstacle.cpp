// Fill out your copyright notice in the Description page of Project Settings.

#include "Level_Class_LevelObstacle.h"
#include "Level_Class_LevelBox.h"
#include "GameInstance_Class.h"
#include "Player_Class_MovementShift.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

ALevel_Class_LevelObstacle::ALevel_Class_LevelObstacle()
{
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetMobility(EComponentMobility::Static);
	StaticMesh->SetGenerateOverlapEvents(false);
	StaticMesh->SetAbsolute(false, false, true);

	ObstacleCollider2D = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider2D"));
	ObstacleCollider2D->SetupAttachment(RootComponent);
	ObstacleCollider2D->SetMobility(EComponentMobility::Static);	
	ObstacleCollider2D->SetGenerateOverlapEvents(true);
	ObstacleCollider2D->SetAbsolute(false, false, true);

	ObstacleCollider3D = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider3D"));
	ObstacleCollider3D->SetupAttachment(RootComponent);
	ObstacleCollider3D->SetMobility(EComponentMobility::Static);
	ObstacleCollider3D->SetGenerateOverlapEvents(false);
	ObstacleCollider3D->SetAbsolute(false, false, true);

	StandingOnTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("StandingOnTrigger"));
	StandingOnTrigger->SetupAttachment(RootComponent);
	StandingOnTrigger->SetMobility(EComponentMobility::Static);
	StandingOnTrigger->SetAbsolute(false, false, true);
}

void ALevel_Class_LevelObstacle::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetWorld())
	{
		GI = Cast<UGameInstance_Class>(GetGameInstance());
	}
}

void ALevel_Class_LevelObstacle::BeginPlay()
{
	Super::BeginPlay();
	
	ParentLevelBox = Cast<ALevel_Class_LevelBox>(GetAttachParentActor());

	if (ParentLevelBox == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s LevelObstacle does not have a parent LevelBox"), *this->GetName());
	}
	else
	{
		//Performing initial setting due to world starting in 2D
		//If bIsPlayerInBox, activate the 2D collider. If not, activate the 3D one. Since the colliders only change profile name when the player
		//is in a Level Box IN 2D, obstacles in other level boxes must be set to 3D initially.
		if (ParentLevelBox->bIsPlayerInBox)
		{
			ObstacleCollider2D->SetCollisionProfileName(TEXT("BlockAllDynamic"));
			ObstacleCollider3D->SetCollisionProfileName(TEXT("NoCollision"));
		}
		else
		{
			ObstacleCollider2D->SetCollisionProfileName(TEXT("NoCollision"));
			ObstacleCollider3D->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		}
		
		//This trigger NEVER changes its profile name
		StandingOnTrigger->SetCollisionProfileName(TEXT("Trigger"));
	}

	StandingOnTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnTriggerBeginOverlap);
	StandingOnTrigger->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnTriggerEndOverlap);
	ObstacleCollider2D->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnColliderEndOverlap);

	Player = Cast<APlayer_Class_MovementShift>(GetWorld()->GetFirstPlayerController()->GetCharacter());
}

void ALevel_Class_LevelObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevel_Class_LevelObstacle::DoSwapDimensionAction(bool bIsIn3D)
{
	//2D = ObstacleCollider2D blocks player and enemies; ObstacleCollider3D has no collision
	//3D = ObstacleCollider3D blocks player and enemies; ObstacleCollider2D has no collision
	if (bIsIn3D)
	{
		ObstacleCollider2D->SetCollisionProfileName(TEXT("NoCollision"));
		ObstacleCollider3D->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	}
	else
	{
		ObstacleCollider2D->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		ObstacleCollider3D->SetCollisionProfileName(TEXT("NoCollision"));

		//This will most likely be replaced with an TArray instead of Player because enemies have to follow this rule too.
		if (bIsPlayerInside)
		{
			if (Player != nullptr)
			{
				CheckAndMoveActorToBaseline(Player);
			}
		}
	}
}

void ALevel_Class_LevelObstacle::SubscribeSwapMethodToGameInstance(bool bShouldAdd)
{
	if (bShouldAdd)
		GI->OnDimensionSwapped.AddDynamic(this, &ALevel_Class_LevelObstacle::DoSwapDimensionAction);
	else
		GI->OnDimensionSwapped.RemoveDynamic(this, &ALevel_Class_LevelObstacle::DoSwapDimensionAction);
}

void ALevel_Class_LevelObstacle::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		bIsPlayerInside = true;

		//Why does player use 'int' instead of a 'bool'? Because the player might be inside multiple level obstacle triggers at once.
		//This is important in ensuring that the player doesn't accidentally teleport to the level box's baseline instead of one of the
		//level obstacle's baseline.
		if (Player != nullptr)
			Player->noOfOverlappingObstacleTrigs++;

		if (GI && !GI->bIsIn3D)
		{
			CheckAndMoveActorToBaseline(OtherActor);
		}	
	}
}

void ALevel_Class_LevelObstacle::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		bIsPlayerInside = false;

		if (Player != nullptr)
			Player->noOfOverlappingObstacleTrigs--;

		//If the player leaves this trigger and is not overlapping anymore triggers
		//then return the player back to the level box's baselineYPos
		if (GI && !GI->bIsIn3D && Player->noOfOverlappingObstacleTrigs == 0)
		{
			OtherActor->SetActorLocation(FVector(OtherActor->GetActorLocation().X, ParentLevelBox->baselineYPos, OtherActor->GetActorLocation().Z));
		}
	}
}

//This method is required to circumvent the player being stuck inside a LevelObstacle and not being able to see through it
void ALevel_Class_LevelObstacle::OnColliderEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		//Same reason as OnTriggerEndOverlap. We want the Collider to only call this when the world is in 2D
		if (GI && !GI->bIsIn3D)
		{
			ObstacleCollider2D->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		}

		//If the player is behind the level obstacle, set opacity material param to 1.0f (no transparency))
		if (Player->GetActorLocation().Y < GetActorLocation().Y)
		{
			StaticMesh->SetScalarParameterValueOnMaterials(TEXT("Opacity"), 1.0f);
		}
	}
}

void ALevel_Class_LevelObstacle::CheckAndMoveActorToBaseline(AActor* ChosenActor)
{
	//If the ChosenActor cannot be moved to the location,
	if (!(ChosenActor->SetActorLocation(FVector(ChosenActor->GetActorLocation().X, obstacleBaselineYPos, ChosenActor->GetActorLocation().Z), true)))
	{
		//Don't move the actor and set the obstacle to overlap the actor only, not block
		ObstacleCollider2D->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

		//If the player is behind the level obstacle, set opacity material param to 0.65f (translucent))
		if (ChosenActor->GetActorLocation().Y < GetActorLocation().Y)
		{
			StaticMesh->SetScalarParameterValueOnMaterials(TEXT("Opacity"), 0.65f);
		}
	}
}

