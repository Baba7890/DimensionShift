#include "Projectile_Class_ProjBase.h"
#include "Player_Class_MovementShift.h"
#include "GameInstance_Class.h"

AProjectile_Class_ProjBase::AProjectile_Class_ProjBase()
{
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
	threeDimenVelocity = FVector::ZeroVector;
}

void AProjectile_Class_ProjBase::BeginPlay()
{
	Super::BeginPlay();

	Player = Cast<APlayer_Class_MovementShift>(UGameplayStatics::GetPlayerCharacter(this, 0));
	Player->OnDimensionSwapCallback.AddDynamic(this, &AProjectile_Class_ProjBase::OnDimensionSwap);

	GetWorldTimerManager().SetTimer(ProjLifeSpanTimerHandle, this, &AProjectile_Class_ProjBase::DestroyProjectile, InitialLifeSpan, false);
	SetLifeSpan(0.0f);
}

void AProjectile_Class_ProjBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile_Class_ProjBase::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag("Enemy"))
	{
		UE_LOG(LogTemp, Log, TEXT("Enemy is shot."));
	}
	else if (OtherActor != nullptr && OtherActor != this && Player != nullptr && ((OtherComp->ComponentHasTag(TEXT("Obstacle3D")) 
		&& Player->bIsIn3D) || (OtherComp->ComponentHasTag(TEXT("Obstacle2D")) && !Player->bIsIn3D)))
	{
		GetWorldTimerManager().ClearTimer(ProjLifeSpanTimerHandle);
		DestroyProjectile();
	}
}

void AProjectile_Class_ProjBase::OnDimensionSwap(float swapDuration)
{
	if (!Player->bIsIn3D)
	{
		FVector ProjVelocity = ProjectileMovement->Velocity;

		//Finding angle between 'forward vector' and 'projectile movement normalized vector' on xy plane
		FVector ZeroZProjVelocity = ProjVelocity;
		ZeroZProjVelocity.Z = 0.0f;
		ZeroZProjVelocity = ZeroZProjVelocity.GetSafeNormal();

		float dotProd = FVector::DotProduct(ZeroZProjVelocity, FVector::ForwardVector);
		float angleBetweenVectors = FMath::Acos(dotProd);
		angleBetweenVectors = FMath::RadiansToDegrees(angleBetweenVectors);

		if (angleBetweenVectors < dissapateAngle || angleBetweenVectors > 180.0f - dissapateAngle)
		{
			//Store 3D velocity and Y position
			threeDimenVelocity = ProjectileMovement->Velocity;
			threeDimenYPosition = GetActorLocation().Y;

			ProjVelocity.Y = 0.0f;
			ProjVelocity = ProjVelocity.GetSafeNormal();
			ProjectileMovement->Velocity = ProjVelocity;
		}
		else
		{
			GetWorldTimerManager().ClearTimer(ProjLifeSpanTimerHandle);
			DestroyProjectile();
		}
	}
	else
	{
		if (threeDimenVelocity == FVector::ZeroVector)
			threeDimenVelocity = ProjectileMovement->Velocity;

		ProjectileMovement->Velocity = FVector::ZeroVector;

		if (threeDimenYPosition != 0.0f)
			SetActorLocation(FVector(GetActorLocation().X, threeDimenYPosition, GetActorLocation().Z));
	}

	GetWorldTimerManager().PauseTimer(ProjLifeSpanTimerHandle);
	GetWorldTimerManager().SetTimer(ProjStopTimerHandle, this, &AProjectile_Class_ProjBase::OnDimensionSwapEnd, swapDuration, false);
}

void AProjectile_Class_ProjBase::OnDimensionSwapEnd()
{
	if (!Player->bIsIn3D)
	{
		ProjectileMovement->Velocity *= ProjectileMovement->InitialSpeed;
		UGameInstance_Class* GI = Cast<UGameInstance_Class>(GetWorld()->GetGameInstance());

		if (GI != nullptr)
			SetActorLocation(FVector(GetActorLocation().X, GI->GetPlayerInLevelBoxBaseline(), GetActorLocation().Z));
	}
	else
	{
		ProjectileMovement->Velocity = threeDimenVelocity;
	}

	GetWorldTimerManager().UnPauseTimer(ProjLifeSpanTimerHandle);
}

void AProjectile_Class_ProjBase::DestroyProjectile()
{
	Player->OnDimensionSwapCallback.RemoveDynamic(this, &AProjectile_Class_ProjBase::OnDimensionSwap);
	Destroy();
}

