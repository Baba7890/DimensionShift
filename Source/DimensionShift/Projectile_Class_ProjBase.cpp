// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile_Class_ProjBase.h"
#include "Player_Class_MovementShift.h"

// Sets default values
AProjectile_Class_ProjBase::AProjectile_Class_ProjBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereCollider = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollider"));
	SetRootComponent(SphereCollider);
	SphereCollider->InitSphereRadius(2.0f);
	SphereCollider->SetCollisionProfileName(TEXT("Trigger"));
	SphereCollider->OnComponentBeginOverlap.AddDynamic(this, &AProjectile_Class_ProjBase::OnTriggerBeginOverlap);

	SphereCollider->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.0f));
	SphereCollider->CanCharacterStepUpOn = ECB_No;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(SphereCollider);
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMesh->CanCharacterStepUpOn = ECB_No;
	StaticMesh->SetGenerateOverlapEvents(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = SphereCollider;
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = 2.0f;
}

// Called when the game starts or when spawned
void AProjectile_Class_ProjBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile_Class_ProjBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile_Class_ProjBase::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayer_Class_MovementShift* Player = Cast<APlayer_Class_MovementShift>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Enemy"))
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy is shot."));
	}
	else if (OtherActor != nullptr && OtherActor != this && OtherComp->ComponentHasTag("Obstacle2D") && Player != nullptr && !Player->bIsIn3D)
	{
		Destroy();
	}
	else if (OtherActor != nullptr && OtherActor != this && OtherComp->ComponentHasTag("Obstacle3D") && Player != nullptr && Player->bIsIn3D)
	{
		Destroy();
	}
}

