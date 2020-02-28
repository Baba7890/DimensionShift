#include "Player_Class_Weapon.h"
#include "Player_Class_MovementShift.h"
#include "Projectile_Class_ProjBase.h"
#include "GameInstance_Class.h"

APlayer_Class_Weapon::APlayer_Class_Weapon()
{
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));
	StaticMesh->CanCharacterStepUpOn = ECB_No;
	StaticMesh->SetGenerateOverlapEvents(false);
	StaticMesh->SetEnableGravity(false);

	ProjectileSpawner = CreateDefaultSubobject<USceneComponent>(TEXT("ProjSpawner"));
	ProjectileSpawner->SetupAttachment(RootComponent);

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("WeaponTrigger"));
	Trigger->SetupAttachment(RootComponent);
	Trigger->InitSphereRadius(4.0f);
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &APlayer_Class_Weapon::OnTriggerBeginOverlap);

	WeaponProjectileComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("WeaponProjMovement"));
	WeaponProjectileComponent->UpdatedComponent = StaticMesh;
	WeaponProjectileComponent->InitialSpeed = baseThrownWeaponSpeed;
	WeaponProjectileComponent->MaxSpeed = 7000.0f;
	WeaponProjectileComponent->bRotationFollowsVelocity = true;
	WeaponProjectileComponent->ProjectileGravityScale = 0.0f;
	WeaponProjectileComponent->SetVelocityInLocalSpace(FVector(0.0f, 0.0f, 0.0f));

	AutoReceiveInput = EAutoReceiveInput::Player0;
}

void APlayer_Class_Weapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetWorld())
	{
		GI = Cast<UGameInstance_Class>(GetGameInstance());

		if (GI == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Game Instance has not been created yet."));
		}
	}
}

void APlayer_Class_Weapon::BeginPlay()
{
	Super::BeginPlay();

}

void APlayer_Class_Weapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsThrowing)
	{
		GetWorldTimerManager().SetTimer(ThrowChargeTimerHandle, this, &APlayer_Class_Weapon::IncrementThrowCharge, throwChargeSpeedInterval, true);
		bIsThrowing = false;
	}

	if (bIsGoingForward)
	{
		FVector NewPosition = GetActorLocation();
		
		if (FVector::Dist(OldPosition, NewPosition) >= throwMaxDistance)
		{
			bIsGoingForward = false;
			
			GetWorldTimerManager().SetTimer(ReturnToPlayerTimerHandle, this, &APlayer_Class_Weapon::ReturnToPlayer, 0.35f, true);

			UE_LOG(LogTemp, Log, TEXT("Changed"));
		}
	}
}

void APlayer_Class_Weapon::FireWeapon()
{
	if (!bIsShooting && PlayerOwner->bHasGun && !bIsThrowing && !PlayerOwner->bIsDashing)
	{
		if (Projectile != NULL)
		{
			if (GetWorld())
			{
				if (GI != nullptr && PlayerOwner->bIsIn3D)
				{
					FRotator SpawnRotation = PlayerOwner->Controller->GetControlRotation();

					FActorSpawnParameters ProjActorSpawnParams;
					ProjActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					GetWorld()->SpawnActor<AProjectile_Class_ProjBase>(Projectile, ProjectileSpawner->GetComponentLocation(), SpawnRotation,
						ProjActorSpawnParams);
					PlayerOwner->ReduceSteam(shootSteamUsage);
					bIsShooting = true;

					GetWorld()->GetTimerManager().SetTimer(FireDelayTimerHandle, this, &APlayer_Class_Weapon::ResetIsShooting, fireDelay, 
						false);
				}
				else if (GI != nullptr && !PlayerOwner->bIsIn3D)
				{
					FVector WorldMousePosition = FVector::ZeroVector;
					FVector WorldMouseDirection = FVector::ZeroVector;

					APlayerController* PlayerController = Cast<APlayerController>(PlayerOwner->Controller);

					if (PlayerController != nullptr)
					{
						PlayerController->DeprojectMousePositionToWorld(WorldMousePosition, WorldMouseDirection);
						WorldMousePosition.Y = PlayerOwner->GetActorLocation().Y;

						//Gimbal lock is a bitch. So I have to find the angle in quaternions instead and convert to FRotator
						FQuat SpawnQuaternion = (WorldMousePosition - PlayerOwner->GetActorLocation()).ToOrientationQuat();
						FRotator SpawnRotation = SpawnQuaternion.Rotator();

						FActorSpawnParameters ProjActorSpawnParams;
						ProjActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

						GetWorld()->SpawnActor<AProjectile_Class_ProjBase>(Projectile, ProjectileSpawner->GetComponentLocation(), SpawnRotation,
							ProjActorSpawnParams);
						PlayerOwner->ReduceSteam(shootSteamUsage);
						bIsShooting = true;

						GetWorld()->GetTimerManager().SetTimer(FireDelayTimerHandle, this, &APlayer_Class_Weapon::ResetIsShooting, fireDelay,
							false);
					}
				}
			}
		}
	}
}

void APlayer_Class_Weapon::ResetIsShooting()
{
	bIsShooting = false;
}

void APlayer_Class_Weapon::PrepareToThrowWeapon()
{
	if (PlayerOwner->bHasGun && !PlayerOwner->bIsDashing && !bIsThrowing)
	{
		currentThrowCharge = 0.0f;
		bIsThrowing = true;
	}
}

void APlayer_Class_Weapon::ThrowWeapon()
{
	if (GetWorldTimerManager().IsTimerActive(ThrowChargeTimerHandle))
		GetWorldTimerManager().ClearTimer(ThrowChargeTimerHandle);

	PlayerOwner->bHasGun = false;
	bIsGoingForward = true;

	DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	SetActorRotation(FRotator::ZeroRotator);

	if (PlayerOwner->bIsIn3D)
	{
		FRotator ThrowRotation = PlayerOwner->Controller->GetControlRotation();
		FVector ThrowDirection = ThrowRotation.Vector();

		WeaponProjectileComponent->SetVelocityInLocalSpace(ThrowDirection * (baseThrownWeaponSpeed + currentThrowCharge));
	}
	else
	{
		FVector WorldMousePosition = FVector::ZeroVector;
		FVector WorldMouseDirection = FVector::ZeroVector;

		APlayerController* PlayerController = Cast<APlayerController>(PlayerOwner->Controller);

		if (PlayerController != nullptr)
		{
			PlayerController->DeprojectMousePositionToWorld(WorldMousePosition, WorldMouseDirection);
			WorldMousePosition.Y = PlayerOwner->GetActorLocation().Y;

			FQuat ThrowQuaternion = (WorldMousePosition - PlayerOwner->GetActorLocation()).ToOrientationQuat();
			FRotator ThrowRotation = ThrowQuaternion.Rotator();
			FVector ThrowDirection = ThrowRotation.Vector();

			WeaponProjectileComponent->SetVelocityInLocalSpace(ThrowDirection * (baseThrownWeaponSpeed + currentThrowCharge));
		}
	}

	OldPosition = GetActorLocation();

	//After a certain distance (or if something is hit) the weapon comes back to the player while ignoring everything (maybe except enemies?)
	//Player then retrieves weapon and puts it at previous location.
}

void APlayer_Class_Weapon::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	/*
	if (OtherActor != nullptr && OtherActor != this && !bIsGoingForward && OtherActor->ActorHasTag(TEXT("Player")))
	{
		WeaponProjectileComponent->SetVelocityInLocalSpace(FVector(0.0f, 0.0f, 0.0f));
		AttachToComponent(PlayerOwner->GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
		
		GetWorldTimerManager().ClearTimer(ReturnToPlayerTimerHandle);
		FTransform ResetTransform = FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f), GetActorScale());
		SetActorRelativeTransform(ResetTransform);

		UE_LOG(LogTemp, Log, TEXT("Called"));
	}
	*/
}

void APlayer_Class_Weapon::IncrementThrowCharge()
{
	currentThrowCharge += throwChargeAmountByInterval;

	if (baseThrownWeaponSpeed + currentThrowCharge > maxThrowSpeed)
	{
		currentThrowCharge = maxThrowSpeed - baseThrownWeaponSpeed;
		GetWorldTimerManager().ClearTimer(ThrowChargeTimerHandle);
	}
}

void APlayer_Class_Weapon::ReturnToPlayer()
{
	SetActorRotation(FRotator::ZeroRotator);

	FVector DirectionToPlayer = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), PlayerOwner->GetActorLocation());
	WeaponProjectileComponent->SetVelocityInLocalSpace(DirectionToPlayer * maxThrowSpeed);
}

