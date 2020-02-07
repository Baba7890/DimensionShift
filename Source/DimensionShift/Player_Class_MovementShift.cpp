#include "Player_Class_MovementShift.h"
#include "GameInstance_Class.h"

APlayer_Class_MovementShift::APlayer_Class_MovementShift()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	//Not allowing character itself to rotate in the direction the camera is facing in
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;	//Character rotates in the direction it is moving in
	GetCharacterMovement()->JumpZVelocity = 1200.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->GravityScale = 2.8f;
	GetCharacterMovement()->MaxAcceleration = 10000.0f;
	GetCharacterMovement()->GroundFriction = 100.0f;
	GetCharacterMovement()->bEnablePhysicsInteraction = true;

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
			GI->OnDimensionSwapped.AddDynamic(this, &APlayer_Class_MovementShift::DoSwapDimensionAction);
		}
	}
}

// Called when the game starts or when spawned
void APlayer_Class_MovementShift::BeginPlay()
{
	Super::BeginPlay();

	if (GI != nullptr)
	{
		SetActorLocation(FVector(GetActorLocation().X, GI->GetPlayerInLevelBoxBaseline(), GetActorLocation().Z));
	}

	FollowCamera3D->SetActive(false);
	TransitionCamera->SetActive(false);
	Controller->SetIgnoreLookInput(true);
}

// Called every frame
void APlayer_Class_MovementShift::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PerformTransitionCameraMovement(DeltaTime);
}

// Called to bind functionality to input
void APlayer_Class_MovementShift::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayer_Class_MovementShift::MoveForward(float fAxis)
{
	if (bCanPlayerMove)
	{
		if (bIsUsing3DControls)
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
	if (bCanPlayerMove)
	{
		if (bIsUsing3DControls)
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

void APlayer_Class_MovementShift::UseSwapDimensionAbility()
{
	if (GI != nullptr)
	{
		GI->SwapDimensions();
	}
}

void APlayer_Class_MovementShift::DoSwapDimensionAction(bool bIsIn3D)
{
	if (bIsIn3D)
	{
		TransitionCamera->SetActive(true);
		FollowCamera2D->SetActive(false);

		bCanPlayerMove = false;

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

		bCanPlayerMove = false;

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().SetTimer(DimensionTimerHandle, this, &APlayer_Class_MovementShift::TurnTo2D, swapDuration, false);
		}
	}
}

void APlayer_Class_MovementShift::TurnTo3D()
{
	bCanPlayerMove = true;
	currentLerpAlpha = 0.0f;
	bHasFinishedViewLerp = false;

	bIsUsing3DControls = true;

	//I had cases where these two lines crashed, just keeping this here in case
	FollowCamera3D->SetActive(true);
	TransitionCamera->SetActive(false);

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Controller->SetControlRotation(FRotator::ZeroRotator);
	Controller->SetIgnoreLookInput(false);
}

void APlayer_Class_MovementShift::TurnTo2D()
{
	bCanPlayerMove = true;
	currentLerpAlpha = 0.0f;
	bHasFinishedViewLerp = false;

	bIsUsing3DControls = false;

	//I had cases where these two lines crashed, just keeping this here in case
	FollowCamera2D->SetActive(true);
	TransitionCamera->SetActive(false);
	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 3000.0f, 0.0f);
	SetActorRotation(FRotator::ZeroRotator);
	Controller->SetIgnoreLookInput(true);

	//This is located here instead of TurnTo3D() to prevent the 3D camera from showing the previous 3D camera rotation for 1 frame before 
	//the player swapped to 2D. So we set the control rotation to zero beforehand
	Controller->SetControlRotation(FRotator::ZeroRotator);

	if (noOfOverlappingObstacleTrigs == 0)
	{
		FVector NewPos = FVector(GetActorLocation().X, GI->GetPlayerInLevelBoxBaseline(), GetActorLocation().Z);
		SetActorLocation(NewPos);
	}
}

void APlayer_Class_MovementShift::PerformTransitionCameraMovement(float deltaTime)
{
	//2D -> 3D Transition Camera movement
	if (!bCanPlayerMove && !bIsUsing3DControls)
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
	else if (!bCanPlayerMove && bIsUsing3DControls)	//3D -> 2D Transition Camera movement
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

//Since the enemies + projectiles in the game also must stop during the transition, I need to put something in game instance
//LevelBox needs to hide itself AFTER the transition when 3D -> 2D
//LevelBox needs to enable itself BEFORE the transition when 2D -> 3D
