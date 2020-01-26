#include "Level_Class_LevelBox.h"
#include "Object_Class_HelperStatics.h"
#include "GameInstance_Class.h"
#include "Level_Class_LevelObstacle.h"

ALevel_Class_LevelBox::ALevel_Class_LevelBox()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	BoxCollider->SetMobility(EComponentMobility::Static);
	BoxCollider->SetCollisionProfileName(TEXT("Trigger"));
	BoxCollider->SetBoxExtent(FVector(40.0f, 40.0f, 40.0f));

	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ALevel_Class_LevelBox::OnBeginOverlap);	
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelBox::OnEndOverlap);
}

void ALevel_Class_LevelBox::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetWorld())
	{
		GI = Cast<UGameInstance_Class>(GetWorld()->GetGameInstance());

		if (GI != nullptr)
		{
			GI->AddLevelBox(this);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s can't get a ref of Game Instance from PostInitializeComponents()"), *this->GetName());
		}
	}

	GetAttachedActors(AttachedObjects);

	if (AttachedObjects.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has no attached actors. Attach a floor and LevelObstacles or remove this level box."),
			*this->GetName());
	}
}

// Called when the game starts or when spawned
void ALevel_Class_LevelBox::BeginPlay()
{
	Super::BeginPlay();

	if (!bIsPlayerInBox)
	{
		EnableLevelBox(false);
	}
}

// Called every frame
void ALevel_Class_LevelBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevel_Class_LevelBox::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//Notice the tag condition. Remember the tag delcaration in the player constructor?
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor->ActorHasTag("Player")))
	{
		bIsPlayerInBox = true;

		if (AttachedObjects.Num() > 0)
		{
			if (GI != nullptr)
			{
				for (int i = 0; i < AttachedObjects.Num(); i++)
				{
					ALevel_Class_LevelObstacle* Obstacle = Cast<ALevel_Class_LevelObstacle>(AttachedObjects[i]);

					if (Obstacle != nullptr)
					{
						Obstacle->SubscribeSwapMethodToGameInstance(GI, true);
					}
				}
			}
		}
	}
}

void ALevel_Class_LevelBox::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && OtherActor->ActorHasTag("Player"))
	{
		bIsPlayerInBox = false;

		if (AttachedObjects.Num() > 0)
		{
			if (GI != nullptr)
			{
				for (int i = 0; i < AttachedObjects.Num(); i++)
				{
					ALevel_Class_LevelObstacle* Obstacle = Cast<ALevel_Class_LevelObstacle>(AttachedObjects[i]);

					if (Obstacle != nullptr)
					{
						Obstacle->SubscribeSwapMethodToGameInstance(GI, false);
					}
				}
			}
		}
	}
}

void ALevel_Class_LevelBox::EnableLevelBox(bool bIsIn3D)
{
	UObject_Class_HelperStatics::EnableActor(this, bIsIn3D);

	if (AttachedObjects.Num() > 0)
	{
		for (int i = 0; i < AttachedObjects.Num(); i++)
		{
			UObject_Class_HelperStatics::EnableActor(AttachedObjects[i], bIsIn3D);
		}
	}
}


