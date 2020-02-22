// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player_Class_CustomMoveComponent.generated.h"

UCLASS()
class DIMENSIONSHIFT_API UPlayer_Class_CustomMoveComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Jumping / Falling", meta = (DisplayName = "Double Jump Z Velocity", ClampMin = "0", UIMin = "0"))
		float doubleJumpZVelocity = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Dashing", meta = (ClampMin = "0", UIMin = "0"))
		float dashForce = 2000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Dashing", meta = (ClampMin = "0", UIMin = "0"))
		float dashDuration = 0.6f;

	float maxBrakingFrictionFactor;
	float maxGravityScale;

public:
	bool DoJump(bool bReplayingMoves) override;
	virtual void BeginPlay() override;
};
