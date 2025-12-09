// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"

#include "ShooterGameState.h"
#include "ShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	auto GS  = GetGameState<AShooterGameState>();
	GS->IncrementTeamScore(TeamByte);
}
