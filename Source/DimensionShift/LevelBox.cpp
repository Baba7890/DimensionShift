// Fill out your copyright notice in the Description page of Project Settings.

#include "LevelBox.h"
#include "HelperStatics.h"
#include "DimensionShiftGameInstance.h"

// Sets default values
ALevelBox::ALevelBox()
{
	PrimaryActorTick.bCanEverTick = true;

	BoxCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collider"));
	BoxCollider->SetMobility(EComponentMobility::Static);
	BoxCollider->SetCollisionProfileName(TEXT("Trigger"));
	BoxCollider->SetBoxExtent(FVector(40.0f, 40.0f, 40.0f));

	BoxCollider->OnComponentBeginOverlap.AddDynamic(this, &ALevelBox::OnBeginOverlap);
	BoxCollider->OnComponentEndOverlap.AddDynamic(this, &ALevelBox::OnEndOverlap);
}

// Called when the game starts or when spawned
void ALevelBox::BeginPlay()
{
	Super::BeginPlay();
	
	UDimensionShiftGameInstance* DSGI = Cast<UDimensionShiftGameInstance>(GetGameInstance());

	if (DSGI != nullptr)
	{
		DSGI->AddLevelBox(this);
	}

	GetAttachedActors(AttachedObjects);

	//Not sure whether this code is correct
	//Apparently OnBeginOverlap is already called before BeginPlay() which makes no sense
	if (!bIsPlayerInBox)
	{
		EnableLevelBox(false);
	}
}

// Called every frame
void ALevelBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALevelBox::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherActor->ActorHasTag("Player")))
	{
		bIsPlayerInBox = true;
	}
}

void ALevelBox::OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if ((OtherActor != nullptr) && (OtherActor != this) && OtherActor->ActorHasTag("Player"))
	{
		bIsPlayerInBox = false;
	}
}

void ALevelBox::EnableLevelBox(bool bIsIn3D)
{
	UHelperStatics::EnableActor(this, bIsIn3D);

	if (AttachedObjects.Num() > 0)
	{
		for (int i = 0; i < AttachedObjects.Num(); i++)
		{
			UHelperStatics::EnableActor(AttachedObjects[i], bIsIn3D);
		}
	}
}

