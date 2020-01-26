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
	ObstacleCollider2D->SetGenerateOverlapEvents(false);
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

void ALevel_Class_LevelObstacle::BeginPlay()
{
	Super::BeginPlay();
	
	ParentLevelBox = Cast<ALevel_Class_LevelBox>(GetAttachParentActor());

	if (ParentLevelBox == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("%s does not have a parent LevelBox"), *this->GetName());
	}
	else
	{
		if (ParentLevelBox->bIsPlayerInBox)
		{
			ObstacleCollider2D->SetCollisionProfileName(TEXT("BlockAllDynamic"));
			ObstacleCollider3D->SetCollisionProfileName(TEXT("NoCollision"));
			StandingOnTrigger->SetCollisionProfileName(TEXT("Trigger"));
		}
		else
		{
			ObstacleCollider2D->SetCollisionProfileName(TEXT("NoCollision"));
			ObstacleCollider3D->SetCollisionProfileName(TEXT("BlockAllDynamic"));
			StandingOnTrigger->SetCollisionProfileName(TEXT("NoCollision"));
		}
	}

	StandingOnTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnBeginOverlap);
	StandingOnTrigger->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnEndOverlap);

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
		StandingOnTrigger->SetCollisionProfileName(TEXT("NoCollision"));
	}
	else
	{
		ObstacleCollider2D->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		ObstacleCollider3D->SetCollisionProfileName(TEXT("NoCollision"));
		StandingOnTrigger->SetCollisionProfileName(TEXT("Trigger"));
	}
}

void ALevel_Class_LevelObstacle::SubscribeSwapMethodToGameInstance(UGameInstance_Class* GI, bool bShouldAdd)
{
	if (bShouldAdd)
		GI->OnDimensionSwapped.AddDynamic(this, &ALevel_Class_LevelObstacle::DoSwapDimensionAction);
	else
		GI->OnDimensionSwapped.RemoveDynamic(this, &ALevel_Class_LevelObstacle::DoSwapDimensionAction);
}

void ALevel_Class_LevelObstacle::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		if (Player != nullptr)
		{
			Player->SetActorLocation(FVector(OtherActor->GetActorLocation().X, obstacleBaselineYPos, OtherActor->GetActorLocation().Z));
			Player->bIsOnLevelObstacle = true;
		}
	}
}

void ALevel_Class_LevelObstacle::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (GetWorld())
	{
		//We need the Game Instance's bIsIn3D in this case because setting the StandingOnTrigger to "NoCollision" profile with the player
		//during the 2D->3D transition inside the trigger box calls OnEndOverlap.
		UGameInstance_Class* GI = Cast<UGameInstance_Class>(GetGameInstance());

		if (GI && !GI->bIsIn3D)
		{
			if (Player != nullptr)
			{
				Player->SetActorLocation(FVector(OtherActor->GetActorLocation().X, ParentLevelBox->baselineYPos,
					OtherActor->GetActorLocation().Z));
				Player->bIsOnLevelObstacle = false;
			}
		}
	}
}

