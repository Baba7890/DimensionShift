#include "Player_Class_MovementShift.h"
#include "GameInstance_Class.h"
#include "Player_Class_Weapon.h"
#include "Player_Class_CustomMoveComponent.h"
#include "Level_Class_LevelObstacle.h"

APlayer_Class_MovementShift::APlayer_Class_MovementShift(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UPlayer_Class_CustomMoveComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	//Not allowing character itself to rotate in the direction the camera is facing in
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	CustomMoveComponent = Cast<UPlayer_Class_CustomMoveComponent>(GetCharacterMovement());

	if (CustomMoveComponent == nullptr)
		UE_LOG(LogTemp, Error, TEXT("The player has no CustomMoveComponent."));

	GetCharacterMovement()->bOrientRotationToMovement = true;	//Character rotates in the direction it is moving in
	GetCharacterMovement()->JumpZVelocity = 1200.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->GravityScale = 2.8f;
	GetCharacterMovement()->MaxAcceleration = 10000.0f;
	GetCharacterMovement()->GroundFriction = 100.0f;
	GetCharacterMovement()->FallingLateralFriction = 5.0f;
	GetCharacterMovement()->bEnablePhysicsInteraction = true;

	JumpMaxCount = 2;

	#pragma region Camera Setup

	CameraBoom2D = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom2D"));
	CameraBoom2D->SetupAttachment(RootComponent);
	CameraBoom2D->SocketOffset.Z = 155.0f;
	CameraBoom2D->bUsePawnControlRotation = false;
	CameraBoom2D->TargetArmLength = 1000.0f;
	CameraBoom2D->SetAbsolute(false, true, false);
	CameraBoom2D->SetWorldRotation(FRotator(0.0f, -90.0f, 0.0f));
	CameraBoom2D->bDoCollisionTest = false;

	FollowCamera2D = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera2D"));
	FollowCamera2D->SetupAttachment(CameraBoom2D, USpringArmComponent::SocketName);
	FollowCamera2D->bUsePawnControlRotation = false;
	FollowCamera2D->OrthoWidth = 1920.0f;
	FollowCamera2D->ProjectionMode = ECameraProjectionMode::Orthographic;

	CameraBoom3D = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom3D"));
	CameraBoom3D->SetupAttachment(RootComponent);
	CameraBoom3D->SocketOffset.Z = 155.0f;
	CameraBoom3D->bUsePawnControlRotation = true;
	CameraBoom3D->TargetArmLength = 275.0f;

	FollowCamera3D = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera3D"));
	FollowCamera3D->SetupAttachment(CameraBoom3D, USpringArmComponent::SocketName);
	FollowCamera3D->bUsePawnControlRotation = false;
	FollowCamera3D->FieldOfView = 110.0f;

	TransCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("TransCameraBoom"));
	TransCameraBoom->SetupAttachment(RootComponent);
	TransCameraBoom->bUsePawnControlRotation = false;
	TransCameraBoom->TargetArmLength = transCamTargetArmLength2D;
	TransCameraBoom->SocketOffset.Z = 155.0f;
	TransCameraBoom->SetAbsolute(false, true, false);
	TransCameraBoom->SetWorldRotation(FRotator(0.0f, -90.0f, 0.0f));
	TransCameraBoom->bDoCollisionTest = false;

	TransitionCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TransitionCamera"));
	TransitionCamera->SetupAttachment(TransCameraBoom, USpringArmComponent::SocketName);
	TransitionCamera->bUsePawnControlRotation = false;
	TransitionCamera->FieldOfView = transCamFieldOfView2D;

	#pragma endregion

	Tags.Add(TEXT("Player"));
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 3000.0f, 0.0f);
}

void APlayer_Class_MovementShift::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetWorld())
	{
		GI = Cast<UGameInstance_Class>(GetGameInstance());

		if (GI != nullptr)
		{
			GI->swapDuration = this->swapDuration;
		}
	}
}

void APlayer_Class_MovementShift::BeginPlay()
{
	Super::BeginPlay();

	if (GI != nullptr)
	{
		SetActorLocation(FVector(GetActorLocation().X, GI->GetPlayerInLevelBoxBaseline(), GetActorLocation().Z));
	}

	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn2D"));
	FollowCamera3D->SetActive(false);
	TransitionCamera->SetActive(false);
	Controller->SetIgnoreLookInput(true);	

	if (GetWorld())
	{
		FActorSpawnParameters WeaponActorSpawnParams;
		WeaponActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		Weapon = GetWorld()->SpawnActor<APlayer_Class_Weapon>(WeaponActor, WeaponActorSpawnParams);
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));
		Weapon->PlayerOwner = this;
		Weapon->WeaponProjectileComponent->HomingTargetComponent = RootComponent;
	}
}

void APlayer_Class_MovementShift::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PerformTransitionCameraMovement(DeltaTime);

	if (currentSteam < maxSteam && !bIsCurrentlyRegenSteam)
	{
		GetWorldTimerManager().ClearTimer(SteamTimerHandle);
		GetWorldTimerManager().SetTimer(SteamTimerHandle, this, &APlayer_Class_MovementShift::RegenSteam, steamRegenIntervalInSeconds, 
			true, steamRegenDelayInSeconds);
		bIsCurrentlyRegenSteam = true;
	}
}

void APlayer_Class_MovementShift::UseSwapDimensionAbility()
{
	if (Weapon != nullptr)
	{
		if (!Weapon->bIsShooting && !bIsDashing)
		{
			//Swap player dimensions only
			bIsIn3D = !bIsIn3D;

			//Change level box
			if (GI != nullptr)
				GI->EnableLevelBoxes(bIsIn3D);

			DoSwapDimensionAction();
		}
	}
}

void APlayer_Class_MovementShift::DoSwapDimensionAction()
{
	if (OnDimensionSwapCallback.IsBound())
	{
		OnDimensionSwapCallback.Broadcast(swapDuration);
	}

	PreDimensionSwapVelocity = GetCharacterMovement()->Velocity;
	GetCharacterMovement()->GravityScale = 0.0f;
	GetCharacterMovement()->StopMovementImmediately();

	bCanPlayerMove = false;

	if (bIsIn3D)
	{
		TransitionCamera->SetActive(true);
		FollowCamera2D->SetActive(false);
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn3D"));

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(DimensionTimerHandle, this, &APlayer_Class_MovementShift::TurnTo3D, swapDuration, false);
		}
	}
	else
	{
		TransitionCamera->SetActive(true);
		TransCameraBoom->SetWorldRotation(Controller->GetControlRotation());
		FollowCamera3D->SetActive(false);
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn2D"));

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(DimensionTimerHandle, this, &APlayer_Class_MovementShift::TurnTo2D, swapDuration, false);
		}
	}
}

void APlayer_Class_MovementShift::Jump()
{
	if (!bIsDashing)
		Super::Jump();
}

void APlayer_Class_MovementShift::ReduceSteam(int amount)
{
	currentSteam -= amount;
	bIsCurrentlyRegenSteam = false;

	if (currentSteam < 0)
		currentSteam = 0.0f;
}

bool APlayer_Class_MovementShift::GetIsPlayerLookingRightInTwoDimen()
{
	if (GetActorRotation().Yaw > -90.0f)
	{
		return true;
	}

	return false;
}

void APlayer_Class_MovementShift::MoveForward(float fAxis)
{
	if (!bIsDashing)
	{
		if (bIsIn3D)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);		//Calculate forward direction
			AddMovementInput(Direction, fAxis);
		}
	}
}

void APlayer_Class_MovementShift::MoveRight(float fAxis)
{
	if (!bIsDashing)
	{
		if (bIsIn3D)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);		//Calculate right direction
			AddMovementInput(Direction, fAxis);
		}
		else
		{
			AddMovementInput(FVector(1.0f, 0.0f, 0.0f), fAxis);		//We don't need to calculate like above for 2D.
		}
	}
}

void APlayer_Class_MovementShift::Dash()
{
	if (currentSteam >= dashSteamUsage)
	{
		if (!bIsDashing)
		{
			CustomMoveComponent->BrakingFrictionFactor = 0.0f;
			bIsDashing = true;

			//Is the player moving?
			if (GetInputAxisValue(TEXT("MoveForward3D")) != 0.0f || GetInputAxisValue(TEXT("MoveRight3D")) != 0.0f)
			{ //Player is moving
				if (!bIsIn3D)
				{
					LaunchCharacter(FVector(GetInputAxisValue(TEXT("MoveRight3D")) * CustomMoveComponent->dashForce, 0.0f, 0.0f), true, true);
				}
				else
				{
					FRotator RotationX = Controller->GetControlRotation();
					FRotator YawRotationX(0, RotationX.Yaw, 0);
					FVector DirectionX = FRotationMatrix(YawRotationX).GetUnitAxis(EAxis::X);
					DirectionX = DirectionX * GetInputAxisValue(TEXT("MoveForward3D"));

					FRotator RotationY = Controller->GetControlRotation();
					FRotator YawRotationY(0, RotationY.Yaw, 0);
					FVector DirectionY = FRotationMatrix(YawRotationY).GetUnitAxis(EAxis::Y);
					DirectionY = DirectionY * GetInputAxisValue(TEXT("MoveRight3D"));

					LaunchCharacter(FVector(DirectionX + DirectionY).GetSafeNormal() * CustomMoveComponent->dashForce, true, true);
				}
			}
			else 
			{ //Player is not moving
				if (!bIsIn3D)
				{
					if (GetIsPlayerLookingRightInTwoDimen())
						LaunchCharacter(FVector(1.0f * CustomMoveComponent->dashForce, 0.0f, 0.0f), true, true);
					else
					{
						LaunchCharacter(FVector(-1.0f * CustomMoveComponent->dashForce, 0.0f, 0.0f), true, true);
					}
				}
				else
				{
					LaunchCharacter(FVector(FollowCamera3D->GetForwardVector().X, FollowCamera3D->GetForwardVector().Y, 0.0f).GetSafeNormal()
						* CustomMoveComponent->dashForce, true, true);
				}
			}

			ReduceSteam(dashSteamUsage);
			
			CustomMoveComponent->GravityScale = 0.0f;
			GetWorldTimerManager().SetTimer(DashTimerHandle, this, &APlayer_Class_MovementShift::ResetIsDashing,
				CustomMoveComponent->dashDuration, false);
		}
	}
}

void APlayer_Class_MovementShift::TurnTo3D()
{
	bCanPlayerMove = true;
	currentLerpAlpha = 0.0f;
	bHasFinishedViewLerp = false;

	if (LevelObstaclesInside.Num() > 0)
	{
		LevelObstaclesInside[0]->CheckAndMoveActorToBaseline(this);
	}

	CustomMoveComponent->GravityScale = CustomMoveComponent->maxGravityScale;
	CustomMoveComponent->Velocity = PreDimensionSwapVelocity;

	FollowCamera3D->SetActive(true);
	TransitionCamera->SetActive(false);

	CustomMoveComponent->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Controller->SetControlRotation(FRotator::ZeroRotator);
	Controller->SetIgnoreLookInput(false);
}

void APlayer_Class_MovementShift::TurnTo2D()
{
	bCanPlayerMove = true;
	currentLerpAlpha = 0.0f;
	bHasFinishedViewLerp = false;

	CustomMoveComponent->GravityScale = CustomMoveComponent->maxGravityScale;
	CustomMoveComponent->Velocity = PreDimensionSwapVelocity;

	if (LevelObstaclesInside.Num() > 0)
	{
		LevelObstaclesInside[0]->CheckAndMoveActorToBaseline(this);
	}

	FollowCamera2D->SetActive(true);
	TransitionCamera->SetActive(false);
	
	CustomMoveComponent->RotationRate = FRotator(0.0f, 3000.0f, 0.0f);
	SetActorRotation(FRotator::ZeroRotator);
	Controller->SetIgnoreLookInput(true);

	//This is located here instead of TurnTo3D() to prevent the 3D camera from showing the previous 3D camera rotation for 1 frame before 
	//the player swapped to 3D. So we set the control rotation to zero beforehand
	Controller->SetControlRotation(FRotator::ZeroRotator);

	if (LevelObstaclesInside.Num() == 0)
	{
		FVector NewPos = FVector(GetActorLocation().X, GI->GetPlayerInLevelBoxBaseline(), GetActorLocation().Z);
		SetActorLocation(NewPos);
	}
}

void APlayer_Class_MovementShift::PerformTransitionCameraMovement(float deltaTime)
{
	//2D -> 3D Transition Camera movement
	if (!bCanPlayerMove && bIsIn3D)
	{
		if (!bHasFinishedViewLerp)
		{
			TransitionCamera->SetFieldOfView(FMath::Lerp(transCamFieldOfView2D, transCamFieldOfView3D, currentLerpAlpha));
			TransCameraBoom->TargetArmLength = FMath::Lerp(transCamTargetArmLength2D, transCamTargetArmLength3D, currentLerpAlpha);

			if (currentLerpAlpha == 1.0f)
			{
				bHasFinishedViewLerp = true;
				currentLerpAlpha = 0.0f;
			}

			currentLerpAlpha += deltaTime * (1.0f / (swapDuration / 2.0f));	//Calculation for x seconds between 0.0f and 1.0f
			currentLerpAlpha = FMath::Clamp(currentLerpAlpha, 0.0f, 1.0f);
		}
		else
		{
			TransCameraBoom->SetWorldRotation(FMath::Lerp(FQuat(FRotator(0.0f, -90.0f, 0.0f)), FQuat(FRotator(FRotator::ZeroRotator)),
				currentLerpAlpha));

			currentLerpAlpha += deltaTime * (1.0f / (swapDuration / 2.0f));
			currentLerpAlpha = FMath::Clamp(currentLerpAlpha, 0.0f, 1.0f);
		}
	}
	else if (!bCanPlayerMove && !bIsIn3D)	//3D -> 2D Transition Camera movement
	{
		if (!bHasFinishedViewLerp)
		{
			TransCameraBoom->SetWorldRotation(FMath::Lerp(FQuat(Controller->GetControlRotation()), FQuat(FRotator(0.0f, -90.0f, 0.0f)),
				currentLerpAlpha));

			if (currentLerpAlpha == 1.0f)
			{
				bHasFinishedViewLerp = true;
				currentLerpAlpha = 0.0f;
			}

			currentLerpAlpha += deltaTime * (1.0f / (swapDuration / 2.0f));
			currentLerpAlpha = FMath::Clamp(currentLerpAlpha, 0.0f, 1.0f);
		}
		else
		{
			TransitionCamera->SetFieldOfView(FMath::Lerp(transCamFieldOfView3D, transCamFieldOfView2D, currentLerpAlpha));
			TransCameraBoom->TargetArmLength = FMath::Lerp(transCamTargetArmLength3D, transCamTargetArmLength2D, currentLerpAlpha);

			currentLerpAlpha += deltaTime * (1.0f / (swapDuration / 2.0f));
			currentLerpAlpha = FMath::Clamp(currentLerpAlpha, 0.0f, 1.0f);
		}
	}
}

void APlayer_Class_MovementShift::RegenSteam()
{
	currentSteam += steamRegenAmount;

	if (currentSteam > maxSteam)
	{
		currentSteam = maxSteam;
		GetWorldTimerManager().ClearTimer(SteamTimerHandle);
		bIsCurrentlyRegenSteam = false;
	}	
}

void APlayer_Class_MovementShift::ResetIsDashing()
{
	CustomMoveComponent->StopMovementImmediately();
	CustomMoveComponent->BrakingFrictionFactor = CustomMoveComponent->maxBrakingFrictionFactor;
	CustomMoveComponent->GravityScale = CustomMoveComponent->maxGravityScale;
	bIsDashing = false;
}


