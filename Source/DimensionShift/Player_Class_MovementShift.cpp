#include "Player_Class_MovementShift.h"
#include "GameInstance_Class.h"

APlayer_Class_MovementShift::APlayer_Class_MovementShift()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Setting Capsule Component initial size
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	//Not allowing character itself to rotate in the direction the camera is facing in
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;	//Character rotates in the direction it is moving in
	GetCharacterMovement()->JumpZVelocity = 600.0f;				//Jump height
	GetCharacterMovement()->AirControl = 0.2f;					//Degree of control while airborne
	GetCharacterMovement()->GravityScale = 1.5f;				//Gravity amount

	//Creating a SpringArmComponent on this Actor

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));	//The parameter names the SpringArmComponent in the editor
	CameraBoom->SetupAttachment(RootComponent);		//Set the spring arm as the root component
	CameraBoom->SocketOffset.Z = 120.0f;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);		//Attaches camera component to spring arm, specifically at the end of the spring arm
	FollowCamera->bUsePawnControlRotation = false;		//Will the camera use the Pawn's controller's rotation?
														//This script inherits ACharacter, which inherits APawn
	FollowCamera->FieldOfView = 110.0f;
	FollowCamera->OrthoWidth = 1560.0f;

	Tags.Add(TEXT("Player"));			//This is a tag declaration for this Actor. Similar to Unity.

	//--------------Code below is for starting in 3D-------------------------
	/*
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 275.0f;
	bIsUsing3DControls = true;
	*/
	//------------------------------------------------------------------------

	//--------------Code below is for starting in 2D-------------------------
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 3000.0f, 0.0f);		//Controls turn speed of character

	const FRotator newRot(0.0f, twoDimensionYaw, 0.0f);

	CameraBoom->bUsePawnControlRotation = false;			//Will the spring arm use the Pawn's controller's rotation?
	CameraBoom->TargetArmLength = 1000.0f;
	CameraBoom->SetAbsolute(false, true, false);			//Required if you want to rotate this Actor according to world rotation
	CameraBoom->SetWorldRotation(newRot);					//Actually sets this Actor's world rotation

	FollowCamera->ProjectionMode = ECameraProjectionMode::Orthographic;
	//------------------------------------------------------------------------
}

// Called when the game starts or when spawned
void APlayer_Class_MovementShift::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance_Class* GI = Cast<UGameInstance_Class>(GetGameInstance());	//Setting GameInstance (singleton). Don't do this in constructor.

	if (GI != nullptr)
	{
		GI->OnDimensionSwapped.AddDynamic(this, &APlayer_Class_MovementShift::DoSwapDimensionAction);	//Delegate subscription
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

//I am kind of confused at these next two functions' code.
//I got it from a tutorial and I vaguely understand only.
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
	UGameInstance_Class* GI = Cast<UGameInstance_Class>(GetGameInstance());

	if (GI != nullptr)
	{
		GI->SwapDimensions();
	}
}

//The method connected to the Game Instance's FOnDimensionSwapped delegate
void APlayer_Class_MovementShift::DoSwapDimensionAction(bool bIsIn3D, float baselineYPos)
{
	if (bIsIn3D)
	{
		TurnTo3D();
	}
	else
	{
		TurnTo2D(baselineYPos);
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

void APlayer_Class_MovementShift::TurnTo2D(float baselineYPos)
{
	bIsUsing3DControls = false;

	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->TargetArmLength = 1000.0f;
	CameraBoom->SetAbsolute(false, true, false);

	FollowCamera->ProjectionMode = ECameraProjectionMode::Orthographic;

	SetActorRotation(FRotator::ZeroRotator);

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 3000.0f, 0.0f);
	Controller->SetIgnoreLookInput(true);

	//moving character to 2D baseline. Only involves moving the Y Position of this actor
	FVector NewPos = FVector(GetActorLocation().X, baselineYPos, GetActorLocation().Z);
	SetActorLocation(NewPos);
}

