#include "Level_Class_LevelBox.h"
#include "Object_Class_HelperStatics.h"
#include "GameInstance_Class.h"

ALevel_Class_LevelBox::ALevel_Class_LevelBox()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	BoxCollider->SetMobility(EComponentMobility::Static);		//Similar to rigidbody's collision type or something
	BoxCollider->SetCollisionProfileName(TEXT("Trigger"));		//Similar to Unity's collision matrix in a way
	BoxCollider->SetBoxExtent(FVector(40.0f, 40.0f, 40.0f));

	//Add the being and end overlap functions to OnComponentBeginOverlap and OnComponentEndOverlap so they will be called on overlap
	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ALevel_Class_LevelBox::OnBeginOverlap);	
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelBox::OnEndOverlap);
}

// Called when the game starts or when spawned
void ALevel_Class_LevelBox::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance_Class* GI = Cast<UGameInstance_Class>(GetGameInstance());

	if (GI != nullptr)
	{
		GI->AddLevelBox(this);
	}

	GetAttachedActors(AttachedObjects);		//This gets all attached actors of this actor. (You can see these actors on the World Outliner)

	//Not sure whether this code is correct
	//Apparently OnBeginOverlap is already called before BeginPlay() which makes no sense
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
	}
}

void ALevel_Class_LevelBox::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && OtherActor->ActorHasTag("Player"))
	{
		bIsPlayerInBox = false;
	}
}

void ALevel_Class_LevelBox::EnableLevelBox(bool bIsIn3D)
{
	//Remember the Helper Class static's EnableActor()?
	UObject_Class_HelperStatics::EnableActor(this, bIsIn3D);

	if (AttachedObjects.Num() > 0)
	{
		for (int i = 0; i < AttachedObjects.Num(); i++)
		{
			UObject_Class_HelperStatics::EnableActor(AttachedObjects[i], bIsIn3D);
		}
	}
}


