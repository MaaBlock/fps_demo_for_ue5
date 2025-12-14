// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UShooterUI;

/**
 *  Simple GameMode for a first person shooter game
 *  Manages game UI
 *  Keeps track of team scores
 */
UCLASS(abstract)
class DEMO_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

public:

};
