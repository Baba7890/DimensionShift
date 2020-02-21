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

	if (bDoesTriggerUseColliderScale)
	{
		StandingOnTrigger->SetWorldScale3D(FVector(ObstacleCollider2D->GetComponentScale().X - 0.3f, ObstacleCollider2D->GetComponentScale().Y,
			ObstacleCollider2D->GetComponentScale().Z + 0.3f));
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

void ALevel_Class_LevelObstacle::DoSwapDimensionAction(bool bIsIn3D, float swapDuration)
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
		{
			if (Player->LevelObstaclesInside.Num() > 0)
			{
				int highestPriority = -1;

				for (int i = 0; i < Player->LevelObstaclesInside.Num(); i++)
				{
					if (Player->LevelObstaclesInside[i] != nullptr)
					{
						if (Player->LevelObstaclesInside[i]->priorityOverPlayerPosition > highestPriority)
						{
							highestPriority = Player->LevelObstaclesInside[i]->priorityOverPlayerPosition;
						}
					}
				}

				priorityOverPlayerPosition = highestPriority + 1;
			}
			else
			{
				priorityOverPlayerPosition = 1;
			}
			
			Player->LevelObstaclesInside.Add(this);

			if (GI && !GI->bIsIn3D)
			{
				CheckAndMoveActorToBaseline(OtherActor);
			}
		}	
	}
}

void ALevel_Class_LevelObstacle::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		bIsPlayerInside = false;

		if (Player != nullptr)
		{
			if (GI && !GI->bIsIn3D)
			{
				//=====================
				//This code only runs in 2D
				if (Player->LevelObstaclesInside.Num() > 1)
				{
					ALevel_Class_LevelObstacle* OtherObstacle = nullptr;

					for (int i = 0; i < Player->LevelObstaclesInside.Num(); i++)
					{
						if (Player->LevelObstaclesInside[i] != nullptr && Player->LevelObstaclesInside[i] != this)
						{
							if (OtherObstacle != nullptr)
							{
								if (Player->LevelObstaclesInside[i]->priorityOverPlayerPosition > OtherObstacle->priorityOverPlayerPosition)
								{
									OtherObstacle = Player->LevelObstaclesInside[i];
								}
							}
							else
							{
								OtherObstacle = Player->LevelObstaclesInside[i];
							}
						}
					}

					if (OtherObstacle->priorityOverPlayerPosition < priorityOverPlayerPosition)
					{
						OtherObstacle->CheckAndMoveActorToBaseline(Player);
					}
				}
				else
				{
					OtherActor->SetActorLocation(FVector(OtherActor->GetActorLocation().X, ParentLevelBox->baselineYPos,
						OtherActor->GetActorLocation().Z));
				}
				//=========================
			}
			
			priorityOverPlayerPosition = -1;
			Player->LevelObstaclesInside.Remove(this);
		}
	}
}

//This method is required to circumvent the player being stuck inside a LevelObstacle and not being able to see through it
void ALevel_Class_LevelObstacle::OnColliderEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Player"))
	{
		SetCollisionProfileNameAndOpacity(TEXT("BlockAllDynamic"), 1.0f);
	}
}

int ALevel_Class_LevelObstacle::SetPriorityOverPlayerPosition()
{
	return priorityOverPlayerPosition;
}

void ALevel_Class_LevelObstacle::CheckAndMoveActorToBaseline(AActor* ChosenActor)
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

	if (!(ChosenActor->SetActorLocation(FVector(ChosenActor->GetActorLocation().X, obstacleBaselineWorldYPos, ChosenActor->GetActorLocation().Z), true)))
	{
		SetCollisionProfileNameAndOpacity(TEXT("OverlapAllDynamic"), 0.65f);
	}
}

void ALevel_Class_LevelObstacle::SetCollisionProfileNameAndOpacity(FName ProfileName, float opacityValue)
{
	if (GI && !GI->bIsIn3D)
	{
		ObstacleCollider2D->SetCollisionProfileName(ProfileName);
	}

	if (Player->GetActorLocation().Y < GetActorLocation().Y)
	{
		StaticMesh->SetScalarParameterValueOnMaterials(TEXT("Opacity"), opacityValue);
	}
}

