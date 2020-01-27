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

		StandingOnTrigger->SetCollisionProfileName(TEXT("Trigger"));
	}

	StandingOnTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnTriggerBeginOverlap);
	StandingOnTrigger->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnTriggerEndOverlap);
	ObstacleCollider2D->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnColliderEndOverlap);

	obstacleBaselineYPos = GetActorLocation().Y;

	Player = Cast<APlayer_Class_MovementShift>(GetWorld()->GetFirstPlayerController()->GetCharacter());
}

// Called every frame
void ALevel_Class_LevelObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevel_Class_LevelObstacle::DoSwapDimensionAction(bool bIsIn3D)
{
	//Is this performance intensive? I don't know.
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

		if (Player != nullptr)
			Player->noOfOverlappingTriggers++;

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
			Player->noOfOverlappingTriggers--;

		if (GI && !GI->bIsIn3D)
		{
			OtherActor->SetActorLocation(FVector(OtherActor->GetActorLocation().X, ParentLevelBox->baselineYPos, OtherActor->GetActorLocation().Z));
		}
	}
}

void ALevel_Class_LevelObstacle::OnColliderEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		//Same reason as OnTriggerEndOverlap. We want the Collider to only call this when the world is in 2D
		if (GI && !GI->bIsIn3D)
		{
			ObstacleCollider2D->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		}

		if (Player->GetActorLocation().Y < GetActorLocation().Y)
		{
			StaticMesh->SetScalarParameterValueOnMaterials(TEXT("Opacity"), 1.0f);
		}
	}
}

void ALevel_Class_LevelObstacle::CheckAndMoveActorToBaseline(AActor* ChosenActor)
{
	if (!(ChosenActor->SetActorLocation(FVector(ChosenActor->GetActorLocation().X, obstacleBaselineYPos, ChosenActor->GetActorLocation().Z), true)))
	{
		ObstacleCollider2D->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

		if (ChosenActor->GetActorLocation().Y < GetActorLocation().Y)
		{
			StaticMesh->SetScalarParameterValueOnMaterials(TEXT("Opacity"), 0.65f);
		}
	}
}

