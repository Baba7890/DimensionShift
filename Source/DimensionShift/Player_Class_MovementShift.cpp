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
	CameraBoom3D->SocketOffset.Z = 120.0f;
	CameraBoom3D->bUsePawnControlRotation = true;
	CameraBoom3D->TargetArmLength = 275.0f;

	FollowCamera3D = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera3D"));
	FollowCamera3D->SetupAttachment(CameraBoom3D, USpringArmComponent::SocketName);
	FollowCamera3D->bUsePawnControlRotation = false;
	FollowCamera3D->FieldOfView = 110.0f;

	FollowCamera3D->SetActive(false);

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

	Controller->SetIgnoreLookInput(true);
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

	//This here might not be as stable as I thought.
	//The way this works is if I disable the current camera, the game will use AActor::CalcCamera() to find the next first camera component
	//and set that camera as the new current camera. Sometimes though, it crashes...
	FollowCamera3D->SetActive(true);
	FollowCamera2D->SetActive(false);

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Controller->SetControlRotation(FRotator::ZeroRotator);
	Controller->SetIgnoreLookInput(false);
}

void APlayer_Class_MovementShift::TurnTo2D()
{
	bIsUsing3DControls = false;

	//This here might not be as stable as I thought.
	FollowCamera2D->SetActive(true);
	FollowCamera3D->SetActive(false);
	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 3000.0f, 0.0f);
	SetActorRotation(FRotator::ZeroRotator);
	Controller->SetIgnoreLookInput(true);

	if (noOfOverlappingObstacleTrigs == 0)
	{
		FVector NewPos = FVector(GetActorLocation().X, GI->GetPlayerInLevelBoxBaseline(), GetActorLocation().Z);
		SetActorLocation(NewPos);
	}
}

