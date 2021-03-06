#include "Player_Class_Weapon.h"
#include "Player_Class_MovementShift.h"
#include "Projectile_Class_ProjBase.h"
#include "GameInstance_Class.h"
#include "Level_Class_LevelObstacle.h"

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

	AutoReceiveInput = EAutoReceiveInput::Player0;
	Tags.Add(TEXT("Weapon"));
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

	if (bIsBeginningThrow)
	{
		GetWorldTimerManager().SetTimer(ThrowChargeTimerHandle, this, &APlayer_Class_Weapon::IncrementThrowCharge, throwChargeSpeedInterval, true);
		bIsBeginningThrow = false;
		bIsChargingThrow = true;
	}

	if (bIsGoingForward)
	{
		FVector NewPosition = GetActorLocation();
		
		if (FVector::Dist(OldPosition, NewPosition) >= currentThrowDistance)
		{
			GetWorldTimerManager().SetTimer(ReturnToPlayerTimerHandle, this, &APlayer_Class_Weapon::ReturnToPlayer, 0.05f, true);
			bIsGoingForward = false;
		}
	}
}

void APlayer_Class_Weapon::FireWeapon()
{
	if (!bIsShooting && PlayerOwner->bHasGun && !bIsChargingThrow && !PlayerOwner->bIsDashing)
	{
		if (shootSteamUsage <= PlayerOwner->currentSteam)
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
}

void APlayer_Class_Weapon::ResetIsShooting()
{
	bIsShooting = false;
}

void APlayer_Class_Weapon::PrepareToThrowWeapon()
{
	if (PlayerOwner->bHasGun && !PlayerOwner->bIsDashing && !bIsBeginningThrow)
	{
		currentThrowCharge = WeaponProjectileComponent->InitialSpeed;
		currentThrowDistance = initialThrowDistance;
		bIsBeginningThrow = true;
	}
}

void APlayer_Class_Weapon::ThrowWeapon()
{
	if (bIsChargingThrow)
	{
		if (GetWorldTimerManager().IsTimerActive(ThrowChargeTimerHandle))
			GetWorldTimerManager().ClearTimer(ThrowChargeTimerHandle);

		PlayerOwner->bHasGun = false;
		bIsGoingForward = true;
		bIsChargingThrow = false;

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

			WeaponProjectileComponent->SetVelocityInLocalSpace(ThrowDirection * currentThrowCharge);
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

				WeaponProjectileComponent->SetVelocityInLocalSpace(ThrowDirection * currentThrowCharge);
			}
		}

		UE_LOG(LogTemp, Log, TEXT("%f"), currentThrowDistance);

		OldPosition = GetActorLocation();
	}
}

void APlayer_Class_Weapon::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsGoingForward)
	{
		if (OtherActor != nullptr && OtherActor != this)
		{
			ALevel_Class_LevelObstacle* Obstacle = Cast<ALevel_Class_LevelObstacle>(OtherActor);

			if (Obstacle)
			{
				if (PlayerOwner != nullptr && ((OtherComp->ComponentHasTag(TEXT("Obstacle2D"))
					&& !PlayerOwner->bIsIn3D) || (OtherComp->ComponentHasTag(TEXT("Obstacle3D")) && PlayerOwner->bIsIn3D)))
				{
					bIsGoingForward = false;
					GetWorldTimerManager().SetTimer(ReturnToPlayerTimerHandle, this, &APlayer_Class_Weapon::ReturnToPlayer, 0.05f, true);
				}
			}
			else if (OtherActor->ActorHasTag(TEXT("Enemy")))
			{
				UE_LOG(LogTemp, Log, TEXT("Enemy is stunned"));
				bIsGoingForward = false;
				GetWorldTimerManager().SetTimer(ReturnToPlayerTimerHandle, this, &APlayer_Class_Weapon::ReturnToPlayer, 0.05f, true);
			}
			else if (!OtherActor->ActorHasTag("LevelBox") && !OtherActor->ActorHasTag("Weapon") && !OtherActor->ActorHasTag("Player"))
			{
				bIsGoingForward = false;
				GetWorldTimerManager().SetTimer(ReturnToPlayerTimerHandle, this, &APlayer_Class_Weapon::ReturnToPlayer, 0.05f, true);
			}
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

				ThreeDimenVelocity = FVector::ZeroVector;
				threeDimenYPosition = 0.0f;
			}
		}
		else if (OtherActor != nullptr && OtherActor != this && OtherActor->ActorHasTag(TEXT("Enemy")))
		{
			UE_LOG(LogTemp, Log, TEXT("Enemy is stunned"));
		}
	}
}

void APlayer_Class_Weapon::IncrementThrowCharge()
{
	if (currentThrowCharge < WeaponProjectileComponent->MaxSpeed)
		currentThrowCharge += throwChargeSpeedBuildEachInterval;
	else
		currentThrowCharge = WeaponProjectileComponent->MaxSpeed;

	if (currentThrowDistance < maxThrowDistance)
		currentThrowDistance += throwChargeDistanceBuildEachInterval;
	else
		currentThrowDistance = maxThrowDistance;

	if (WeaponProjectileComponent->InitialSpeed + currentThrowCharge == WeaponProjectileComponent->MaxSpeed && currentThrowDistance
		== maxThrowDistance)
		GetWorldTimerManager().ClearTimer(ThrowChargeTimerHandle);
}

void APlayer_Class_Weapon::ReturnToPlayer()
{
	FVector WorldDirectionToPlayer = UKismetMathLibrary::GetDirectionUnitVector(GetActorLocation(), PlayerOwner->GetActorLocation());
	FVector LocalDirectionToPlayer = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), WorldDirectionToPlayer);

	WeaponProjectileComponent->SetVelocityInLocalSpace(LocalDirectionToPlayer * throwReturnSpeed);
}

void APlayer_Class_Weapon::OnDimensionSwap(float swapDuration)
{
	//We know that weapon is flying when player does not have gun
	if (!PlayerOwner->bHasGun)
	{
		if (!PlayerOwner->bIsIn3D)
		{
			FVector ProjVelocity = WeaponProjectileComponent->Velocity;

			FVector ZeroZProjVelocity = ProjVelocity;
			ZeroZProjVelocity.Z = 0.0f;
			ZeroZProjVelocity = ZeroZProjVelocity.GetSafeNormal();

			float dotProd = FVector::DotProduct(ZeroZProjVelocity, FVector::ForwardVector);
			float angleBetweenVectors = FMath::Acos(dotProd);
			angleBetweenVectors = FMath::RadiansToDegrees(angleBetweenVectors);

			if (bIsGoingForward)
			{
				if (angleBetweenVectors < dissapateAngle || angleBetweenVectors > 180.0f - dissapateAngle)
				{
					ThreeDimenVelocity = ProjVelocity;
					threeDimenYPosition = GetActorLocation().Y;

					ProjVelocity.Y = 0.0f;
					ProjVelocity = ProjVelocity.GetSafeNormal();
					WeaponProjectileComponent->Velocity = ProjVelocity;
				}
				else
				{
					bIsGoingForward = false;
					SetActorLocation(PlayerOwner->GetActorLocation());
				}
			}
			else
			{
				if (angleBetweenVectors < dissapateAngle || angleBetweenVectors > 180.0f - dissapateAngle)
				{
					threeDimenYPosition = GetActorLocation().Y;

					GetWorldTimerManager().PauseTimer(ReturnToPlayerTimerHandle);
					WeaponProjectileComponent->StopMovementImmediately();
				}
				else
				{
					SetActorLocation(PlayerOwner->GetActorLocation());
				}
			}
		}
		else
		{
			if (bIsGoingForward)
			{
				if (ThreeDimenVelocity == FVector::ZeroVector)
					ThreeDimenVelocity = WeaponProjectileComponent->Velocity;
			}
			else
			{
				GetWorldTimerManager().PauseTimer(ReturnToPlayerTimerHandle);
			}
				
			WeaponProjectileComponent->StopMovementImmediately();

			if (threeDimenYPosition != 0.0f)
				SetActorLocation(FVector(GetActorLocation().X, threeDimenYPosition, GetActorLocation().Z));
		}

		GetWorldTimerManager().SetTimer(WeaponStopTimerHandle, this, &APlayer_Class_Weapon::OnDimensionSwapEnd, swapDuration, false);
	}
}

void APlayer_Class_Weapon::OnDimensionSwapEnd()
{
	if (!PlayerOwner->bIsIn3D)
	{
		if (GI != nullptr)
		{
			//If the player has the gun already, the weapon's flying angle was more than dissapateAngle and it has teleported back to player
			//So do not call this. Otherwise, yes.
			if (!PlayerOwner->bHasGun)
				SetActorLocation(FVector(GetActorLocation().X, GI->GetPlayerInLevelBoxBaseline(), GetActorLocation().Z));
		}

		if (bIsGoingForward)
			WeaponProjectileComponent->Velocity *= (WeaponProjectileComponent->InitialSpeed + currentThrowCharge);
		else
			GetWorldTimerManager().UnPauseTimer(ReturnToPlayerTimerHandle);
	}
	else
	{
		if (bIsGoingForward)
			WeaponProjectileComponent->Velocity = ThreeDimenVelocity;
		else
			GetWorldTimerManager().UnPauseTimer(ReturnToPlayerTimerHandle);
	}
}



