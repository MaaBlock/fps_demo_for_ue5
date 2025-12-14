// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DemoPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurrentCoinChanged,int32,CurrentCoin);

/**
 * 
 */
UCLASS()
class DEMO_API ADemoPlayerState : public APlayerState
{
	GENERATED_BODY()
protected:
	UPROPERTY(ReplicatedUsing="OnRep_CurrentCoin")
	int32 CurrentCoin;
public:
	FOnCurrentCoinChanged OnCurrentCoinChanged;
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	void OnRep_CurrentCoin();
public:
	void Auth_AddCoins(int32 Coins);
	int32 GetCurrentCoin();
};
