// Fill out your copyright notice in the Description page of Project Settings.

#include "MainPlayer.h"
#include "DimensionShiftGameInstance.h"

// Sets default values
AMainPlayer::AMainPlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	//Not allowing character itself to rotate in the direction the camera is facing in
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;				//Character rotates in the direction it is moving in
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->GravityScale = 1.5f;

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

// Called when the game starts or when spawned
void AMainPlayer::BeginPlay()
{
	Super::BeginPlay();
	
	UDimensionShiftGameInstance* DSGI = Cast<UDimensionShiftGameInstance>(GetGameInstance());

	if (DSGI != nullptr)
	{
		DSGI->OnDimensionSwapped.AddDynamic(this, &AMainPlayer::DoSwapDimensionAction);
	}

	//--------------Code below is for starting in 2D-------------------------
	Controller->SetIgnoreLookInput(true);
	//------------------------------------------------------------------------
}

// Called every frame
void AMainPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMainPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMainPlayer::MoveForward(float fAxis)
{
	if (bIsUsing3DControls)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);		//Calculate forward direction
		AddMovementInput(Direction, fAxis);
	}
}

void AMainPlayer::MoveRight(float fAxis)
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
		AddMovementInput(FVector(1.0f, 0.0f, 0.0f), fAxis);
	}
}

void AMainPlayer::UseSwapDimensionAbility()
{
	UDimensionShiftGameInstance* DSGI = Cast<UDimensionShiftGameInstance>(GetGameInstance());

	if (DSGI != nullptr)
	{
		DSGI->SwapDimensions();
	}
}

//The method connected to the Game Instance's FOnDimensionSwapped delegate
void AMainPlayer::DoSwapDimensionAction(bool bIsIn3D, float baselineYPos)
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

void AMainPlayer::TurnTo3D()
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

void AMainPlayer::TurnTo2D(float baselineYPos)
{
	bIsUsing3DControls = false;

	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->TargetArmLength = 1000.0f;
	CameraBoom->SetAbsolute(false, true, false);

	FollowCamera->ProjectionMode = ECameraProjectionMode::Orthographic;

	SetActorRotation(FRotator::ZeroRotator);

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 3000.0f, 0.0f);
	Controller->SetIgnoreLookInput(true);

	//moving character to baseline. Only involves moving the Y Position of this actor
	FVector NewPos = FVector(GetActorLocation().X, baselineYPos, GetActorLocation().Z);
	SetActorLocation(NewPos);
}

