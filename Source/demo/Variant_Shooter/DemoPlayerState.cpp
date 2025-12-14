// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Shooter/DemoPlayerState.h"

#include "demo.h"
#include "Net/UnrealNetwork.h"

void ADemoPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ADemoPlayerState, CurrentCoin, COND_OwnerOnly);
}

void ADemoPlayerState::OnRep_CurrentCoin()
{
	UE_LOG(LogTemp, Warning, TEXT("Current Coins changed to %d"), CurrentCoin);
	OnCurrentCoinChanged.Broadcast(CurrentCoin);
}

void ADemoPlayerState::Auth_AddCoins(int32 Coins)
{
	if (!HasAuthority())
		return;
	CurrentCoin += Coins;
	SV_REPCALL(CurrentCoin);
}

int32 ADemoPlayerState::GetCurrentCoin()
{
	return CurrentCoin;
}
