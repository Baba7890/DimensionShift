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

	ProjectileSpawner = CreateDefaultSubobject<USceneComponent>(TEXT("ProjSpawner"));
	ProjectileSpawner->SetupAttachment(RootComponent);

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

}

void APlayer_Class_Weapon::FireWeapon()
{
	if (!bIsShooting && PlayerOwner->bHasGun)
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

