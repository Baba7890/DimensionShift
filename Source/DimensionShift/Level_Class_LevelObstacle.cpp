// Fill out your copyright notice in the Description page of Project Settings.

#include "Level_Class_LevelObstacle.h"
#include "Level_Class_LevelBox.h"
#include "GameInstance_Class.h"
#include "Player_Class_MovementShift.h"

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
	ObstacleCollider2D->ComponentTags.Add(TEXT("Obstacle2D"));

	ObstacleCollider3D = CreateDefaultSubobject<UBoxComponent>(TEXT("Collider3D"));
	ObstacleCollider3D->SetupAttachment(RootComponent);
	ObstacleCollider3D->SetMobility(EComponentMobility::Static);
	ObstacleCollider3D->SetGenerateOverlapEvents(false);
	ObstacleCollider3D->SetAbsolute(false, false, true);
	ObstacleCollider3D->ComponentTags.Add(TEXT("Obstacle3D"));

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
		ObstacleCollider2D->SetCollisionProfileName(TEXT("BlockAllDynamic2D"));
		ObstacleCollider3D->SetCollisionProfileName(TEXT("BlockAllDynamic3D"));
		StandingOnTrigger->SetCollisionProfileName(TEXT("Trigger"));
	}

	if (bDoesTriggerUseColliderScale)
	{
		StandingOnTrigger->SetWorldScale3D(FVector(ObstacleCollider2D->GetComponentScale().X - 0.3f, ObstacleCollider2D->GetComponentScale().Y,
			ObstacleCollider2D->GetComponentScale().Z + 0.3f));
	}

	StandingOnTrigger->OnComponentBeginOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnTriggerBeginOverlap);
	StandingOnTrigger->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnTriggerEndOverlap);
	ObstacleCollider2D->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnColliderEndOverlap);
	ObstacleCollider2D->OnComponentBeginOverlap.AddDynamic(this, &ALevel_Class_LevelObstacle::OnColliderBeginOverlap);

	Player = Cast<APlayer_Class_MovementShift>(GetWorld()->GetFirstPlayerController()->GetCharacter());
}

void ALevel_Class_LevelObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevel_Class_LevelObstacle::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		/*
		//Why does player use 'int' instead of a 'bool'? Because the player might be inside multiple level obstacle triggers at once.
		//This is important in ensuring that the player doesn't accidentally teleport to the level box's baseline instead of one of the
		//level obstacle's baseline.
		if (Player != nullptr)
		{
			if (Player->ObstacleTriggersInside.Num() > 0)
			{
				int highestPriority = -1;

				for (int i = 0; i < Player->ObstacleTriggersInside.Num(); i++)
				{
					if (Player->ObstacleTriggersInside[i] != nullptr)
					{
						if (Player->ObstacleTriggersInside[i]->priorityOverPlayerPosition > highestPriority)
						{
							highestPriority = Player->ObstacleTriggersInside[i]->priorityOverPlayerPosition;
						}
					}
				}

				priorityOverPlayerPosition = highestPriority + 1;
			}
			else
			{
				priorityOverPlayerPosition = 1;
			}
			
			Player->ObstacleTriggersInside.Add(this);
		}	
		*/
	}
}

void ALevel_Class_LevelObstacle::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		/*
		if (Player != nullptr)
		{
			priorityOverPlayerPosition = -1;
			Player->ObstacleTriggersInside.Remove(this);
		}
		*/
	}
}

void ALevel_Class_LevelObstacle::OnColliderEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		Player->ObstacleCollidersInside.Remove(this);

		if (ObstacleCollider2D->GetCollisionProfileName() == TEXT("OverlapAllDynamic2D"))
		{
			SetCollisionProfileNameAndOpacity(TEXT("BlockAllDynamic2D"), 1.0f);
		}

		if (!Player->bIsIn3D)
		{
			OtherActor->SetActorLocation(FVector(OtherActor->GetActorLocation().X, ParentLevelBox->baselineYPos,
				OtherActor->GetActorLocation().Z), true);
		}
	}
}

void ALevel_Class_LevelObstacle::OnColliderBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		Player->ObstacleCollidersInside.Add(this);
	}
}

int ALevel_Class_LevelObstacle::GetPriorityOverPlayerPosition()
{
	return priorityOverPlayerPosition;
}

float ALevel_Class_LevelObstacle::GetObstacleBaselinePosition()
{
	float obstacleBaselineWorldYPos;

	if (bDoesBaselineUseObstacleCenter)
	{
		obstacleBaselineWorldYPos = GetActorLocation().Y;
	}
	else
	{
		//Converting local location to world location
		obstacleBaselineWorldYPos = ParentLevelBox->GetActorLocation().Y + (obstacleBaselineLocalYPos * ParentLevelBox->GetActorScale3D().Y);
	}

	return obstacleBaselineWorldYPos;
}

void ALevel_Class_LevelObstacle::SetCollisionProfileNameAndOpacity(FName ProfileName, float opacityValue)
{
	if (!Player->bIsIn3D)
	{
		ObstacleCollider2D->SetCollisionProfileName(ProfileName);
	}

	if (Player->GetActorLocation().Y < GetActorLocation().Y)
	{
		StaticMesh->SetScalarParameterValueOnMaterials(TEXT("Opacity"), opacityValue);
	}
}

