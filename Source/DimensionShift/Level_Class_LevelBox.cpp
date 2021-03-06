#include "Level_Class_LevelBox.h"
#include "Object_Class_HelperStatics.h"
#include "GameInstance_Class.h"
#include "Level_Class_LevelObstacle.h"

ALevel_Class_LevelBox::ALevel_Class_LevelBox()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	SetRootComponent(BoxCollider);
	BoxCollider->SetMobility(EComponentMobility::Static);
	BoxCollider->SetCollisionProfileName(TEXT("Trigger"));
	BoxCollider->SetBoxExtent(FVector(40.0f, 40.0f, 40.0f));

	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ALevel_Class_LevelBox::OnBeginOverlap);
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &ALevel_Class_LevelBox::OnEndOverlap);

	Tags.Add(TEXT("LevelBox"));
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

void ALevel_Class_LevelBox::BeginPlay()
{
	Super::BeginPlay();

	if (!bIsPlayerInBox)
	{
		EnableLevelBox(false);
	}
}

void ALevel_Class_LevelBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevel_Class_LevelBox::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
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

void ALevel_Class_LevelBox::DoDimensionSwapAction(bool bPlayerIsIn3D, float swapDuration)
{
	//Is this intensive?
	DimensionTimerDelegate.BindUFunction(this, FName("EnableLevelBox"), bPlayerIsIn3D);
	GetWorld()->GetTimerManager().SetTimer(DimensionTimerHandle, DimensionTimerDelegate, swapDuration / 2.0f, false);
}

void ALevel_Class_LevelBox::EnableLevelBox(bool bPlayerIsIn3D)
{
	UObject_Class_HelperStatics::EnableActor(this, bPlayerIsIn3D);

	if (AttachedObjects.Num() > 0)
	{
		for (int i = 0; i < AttachedObjects.Num(); i++)
		{
			UObject_Class_HelperStatics::EnableActor(AttachedObjects[i], bPlayerIsIn3D);
		}
	}
}


