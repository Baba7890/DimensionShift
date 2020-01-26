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
	GetCharacterMovement()->bEnablePhysicsInteraction = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SocketOffset.Z = 120.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 110.0f;
	FollowCamera->OrthoWidth = 1560.0f;

	Tags.Add(TEXT("Player"));

	//--------------Code below is for starting in 3D-------------------------
	/*
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 275.0f;
	bIsUsing3DControls = true;
	*/
	//------------------------------------------------------------------------

	//--------------Code below is for starting in 2D-------------------------
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 3000.0f, 0.0f);

	const FRotator newRot(0.0f, twoDimensionYaw, 0.0f);

	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->TargetArmLength = 1000.0f;
	CameraBoom->SetAbsolute(false, true, false);
	CameraBoom->SetWorldRotation(newRot);

	FollowCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	//------------------------------------------------------------------------
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

	//--------------Code below is for starting in 2D-------------------------
	Controller->SetIgnoreLookInput(true);
	//------------------------------------------------------------------------
}

// Called every frame
void APlayer_Class_MovementShift::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayer_Class_MovementShift::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void APlayer_Class_MovementShift::MoveForward(float fAxis)
{
	if (bIsUsing3DControls)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);		//Calculate forward direction
		AddMovementInput(Direction, fAxis);
	}
}

void APlayer_Class_MovementShift::MoveRight(float fAxis)
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
		TurnTo3D();
	}
	else
	{
		TurnTo2D();
	}
}

void APlayer_Class_MovementShift::TurnTo3D()
{
	bIsUsing3DControls = true;

	CameraBoom->bUsePawnControlRotation = true;
	Controller->SetControlRotation(FRotator::ZeroRotator);
	CameraBoom->TargetArmLength = 275.0f;
	CameraBoom->SetAbsolute(false, false, false);

	FollowCamera->ProjectionMode = ECameraProjectionMode::Perspective;

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Controller->SetIgnoreLookInput(false);
}

void APlayer_Class_MovementShift::TurnTo2D()
{
	bIsUsing3DControls = false;

	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->TargetArmLength = 1000.0f;
	CameraBoom->SetAbsolute(false, true, false);

	FollowCamera->ProjectionMode = ECameraProjectionMode::Orthographic;

	SetActorRotation(FRotator::ZeroRotator);

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 3000.0f, 0.0f);
	Controller->SetIgnoreLookInput(true);

	FVector NewPos = FVector(GetActorLocation().X, GI->GetPlayerInLevelBoxBaseline(), GetActorLocation().Z);
	SetActorLocation(NewPos);
}

