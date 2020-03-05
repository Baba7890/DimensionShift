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
	WeaponProjectileComponent->InitialSpeed = 1000.0f;
	WeaponProjectileComponent->MaxSpeed = 4000.0f;
	WeaponProjectileComponent->bRotationFollowsVelocity = true;
	WeaponProjectileComponent->ProjectileGravityScale = 0.0f;
	WeaponProjectileComponent->SetVelocityInLocalSpace(FVector(0.0f, 0.0f, 0.0f));
	WeaponProjectileComponent->HomingAccelerationMagnitude = 20.0f;

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
		bIsCharging = true;
	}

	if (bIsGoingForward)
	{
		FVector NewPosition = GetActorLocation();
		
		if (FVector::Dist(OldPosition, NewPosition) >= throwMaxDistance)
		{
			GetWorldTimerManager().SetTimer(ReturnToPlayerTimerHandle, this, &APlayer_Class_Weapon::ReturnToPlayer, 0.05f, true);
			bIsGoingForward = false;
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
					FHitResult Hit;
					FVector CameraWorldLocation = PlayerOwner->FollowCamera3D->GetComponentLocation();
					FVector CameraForwardVector = PlayerOwner->FollowCamera3D->GetForwardVector();

					bool vectorSelect = GetWorld()->LineTraceSingleByChannel(Hit, CameraWorldLocation, 
						CameraWorldLocation + CameraForwardVector * 4000.0f, ECollisionChannel::ECC_GameTraceChannel3);

					FVector ProjMoveDirection = UKismetMathLibrary::SelectVector(Hit.ImpactPoint, Hit.TraceEnd, vectorSelect);
					FRotator SpawnRotation = UKismetMathLibrary::FindLookAtRotation(ProjectileSpawner->GetComponentLocation(), ProjMoveDirection);

					FActorSpawnParameters ProjActorSpawnParams;
					ProjActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					GetWorld()->SpawnActor<AProjectile_Class_ProjBase>(Projectile, ProjectileSpawner->GetComponentLocation(), SpawnRotation,
						ProjActorSpawnParams);
					PlayerOwner->ReduceSteam(shootSteamUsage);
					bIsShooting = true;

					GetWorld()->GetTimerManager().SetTimer(FireDelayTimerHandle, this, &APlayer_Class_Weapon::ResetIsShooting, shootFireDelay, 
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

						GetWorld()->GetTimerManager().SetTimer(FireDelayTimerHandle, this, &APlayer_Class_Weapon::ResetIsShooting, shootFireDelay,
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
	if (bIsCharging)
	{
		if (GetWorldTimerManager().IsTimerActive(ThrowChargeTimerHandle))
			GetWorldTimerManager().ClearTimer(ThrowChargeTimerHandle);

		PlayerOwner->bHasGun = false;
		bIsGoingForward = true;
		bIsCharging = false;

		DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		SetActorRotation(FRotator::ZeroRotator);

		if (PlayerOwner->bIsIn3D)
		{
			FHitResult Hit;
			FVector CameraWorldLocation = PlayerOwner->FollowCamera3D->GetComponentLocation();
			FVector CameraForwardVector = PlayerOwner->FollowCamera3D->GetForwardVector();

			bool vectorSelect = GetWorld()->LineTraceSingleByChannel(Hit, CameraWorldLocation,
				CameraWorldLocation + CameraForwardVector * 4000.0f, ECollisionChannel::ECC_GameTraceChannel3);

			FVector ProjMoveDirection = UKismetMathLibrary::SelectVector(Hit.ImpactPoint, Hit.TraceEnd, vectorSelect);
			FRotator ThrowRotation = UKismetMathLibrary::FindLookAtRotation(ProjectileSpawner->GetComponentLocation(), ProjMoveDirection);
			FVector ThrowDirection = ThrowRotation.Vector();

			WeaponProjectileComponent->SetVelocityInLocalSpace(ThrowDirection * (WeaponProjectileComponent->InitialSpeed + currentThrowCharge));
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

				WeaponProjectileComponent->SetVelocityInLocalSpace(ThrowDirection * (WeaponProjectileComponent->InitialSpeed + currentThrowCharge));
			}
		}

		OldPosition = GetActorLocation();
	}
}

void APlayer_Class_Weapon::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsGoingForward)
	{
		if (OtherActor != nullptr && OtherActor != this && PlayerOwner != nullptr && ((OtherComp->ComponentHasTag(TEXT("Obstacle2D"))
			&& !PlayerOwner->bIsIn3D) || (OtherComp->ComponentHasTag(TEXT("Obstacle3D")) && PlayerOwner->bIsIn3D)))
		{
			bIsGoingForward = false;
			GetWorldTimerManager().SetTimer(ReturnToPlayerTimerHandle, this, &APlayer_Class_Weapon::ReturnToPlayer, 0.05f, true);
		}
		else if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag(TEXT("Enemy")))
		{
			UE_LOG(LogTemp, Log, TEXT("Enemy is stunned"));
			bIsGoingForward = false;
			GetWorldTimerManager().SetTimer(ReturnToPlayerTimerHandle, this, &APlayer_Class_Weapon::ReturnToPlayer, 0.05f, true);
		}
	}
	else
	{
		if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag(TEXT("Player")))
		{
			if (!IsAttachedTo(OtherActor))
			{
				GetWorldTimerManager().ClearTimer(ReturnToPlayerTimerHandle);
				WeaponProjectileComponent->SetVelocityInLocalSpace(FVector(0.0f, 0.0f, 0.0f));
				AttachToComponent(PlayerOwner->GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));

				FTransform InitialTransform = FTransform(FRotator(0.0f, 0.0f, 0.0f), FVector(0.0f, 0.0f, 0.0f), GetActorScale());
				SetActorRelativeTransform(InitialTransform);
				PlayerOwner->bHasGun = true;
			}
		}
	}
}

void APlayer_Class_Weapon::IncrementThrowCharge()
{
	currentThrowCharge += throwChargeAmountByInterval;

	if (WeaponProjectileComponent->InitialSpeed + currentThrowCharge > WeaponProjectileComponent->MaxSpeed)
	{
		currentThrowCharge = WeaponProjectileComponent->MaxSpeed - WeaponProjectileComponent->InitialSpeed;
		GetWorldTimerManager().ClearTimer(ThrowChargeTimerHandle);
	}
}

void APlayer_Class_Weapon::ReturnToPlayer()
{
	FVector WorldDirectionToPlayer = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), PlayerOwner->GetActorLocation());
	FVector LocalDirectionToPlayer = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), WorldDirectionToPlayer);

	WeaponProjectileComponent->SetVelocityInLocalSpace(LocalDirectionToPlayer * throwReturnSpeed);
}

