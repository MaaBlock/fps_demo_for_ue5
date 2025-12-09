// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Shooter/ShooterGameState.h"

#include "Net/UnrealNetwork.h"


void AShooterGameState::OnRep_CurrentScore()
{
	OnScoreChanged.Broadcast(CurrentScore[0],CurrentScore[1]);
}

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShooterGameState, CurrentScore);
}

AShooterGameState::AShooterGameState() : CurrentScore{}
{
	
}

void AShooterGameState::IncrementTeamScore(uint8 TeamByte)
{
	if (GetLocalRole() != ROLE_Authority)
		return;
	if (TeamByte >= 2)
		return;
	CurrentScore[TeamByte]++;
    OnRep_CurrentScore();
}
