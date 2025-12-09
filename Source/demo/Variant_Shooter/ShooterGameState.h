// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShooterGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreChanged, int, Team1Score, int, Team2Score);
/**
 * 
 */
UCLASS()
class DEMO_API AShooterGameState : public AGameState
{
	GENERATED_BODY()
protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentScore)
	int CurrentScore[2];
public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnScoreChanged OnScoreChanged;
protected:
	UFUNCTION()
	void OnRep_CurrentScore();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	AShooterGameState();
	void IncrementTeamScore(uint8 TeamByte);
};
