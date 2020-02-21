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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement: Jumping / Falling", meta = (DisplayName = "Double Jump Z Velocity", ClampMin = "0", UIMin = "0"))
		float doubleJumpZVelocity = 0.0f;

public:
	bool DoJump(bool bReplayingMoves) override;
};
