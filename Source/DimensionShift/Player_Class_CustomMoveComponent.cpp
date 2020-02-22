// Fill out your copyright notice in the Description page of Project Settings.

#include "Player_Class_CustomMoveComponent.h"
#include "GameFramework/Character.h"
#include "Player_Class_MovementShift.h"

bool UPlayer_Class_CustomMoveComponent::DoJump(bool bReplayingMoves)
{
	if (CharacterOwner && CharacterOwner->CanJump())
	{
		// Don't jump if we can't move up/down.
		if (!bConstrainToPlane || FMath::Abs(PlaneConstraintNormal.Z) != 1.f)
		{
			//If character's jump count is 0, then jump normally
			if (CharacterOwner->JumpCurrentCount == 0)
			{
				Velocity.Z = FMath::Max(Velocity.Z, JumpZVelocity);
				SetMovementMode(MOVE_Falling);
				return true;
			}
			else if (CharacterOwner->JumpCurrentCount == 1) //Otherwise, double jump and use some steam
			{
				APlayer_Class_MovementShift* PlayerOwner = Cast<APlayer_Class_MovementShift>(CharacterOwner);

				if (PlayerOwner != nullptr && PlayerOwner->currentSteam >= PlayerOwner->doubleJumpSteamUsage)
				{
					Velocity.Z = FMath::Max(Velocity.Z, doubleJumpZVelocity);
					SetMovementMode(MOVE_Falling);
					PlayerOwner->ReduceSteam(PlayerOwner->doubleJumpSteamUsage);

					return true;
				}
			}
		}
	}

	return false;
}

void UPlayer_Class_CustomMoveComponent::BeginPlay()
{
	Super::BeginPlay();

	maxBrakingFrictionFactor = BrakingFrictionFactor;
	maxGravityScale = GravityScale;
}
