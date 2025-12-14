// Fill out your copyright notice in the Description page of Project Settings.


#include "Variant_Shooter/UI/GameUI.h"

#include "DemoPlayerState.h"
#include "ShooterCharacter.h"
#include "Utils/FGameUtils.h"

class ADemoPlayerState;

void UGameUI::PostNetReceive()
{
	Super::PostNetReceive();
}

void UGameUI::BindBpToDelegrate()
{
	FGameUtils::SetRetryTimer(
	[this]()
		{
			if (ADemoPlayerState* PS = GetOwningPlayerState<ADemoPlayerState>())
			{
				BP_UpdateCoin(PS->GetCurrentCoin());
				PS->OnCurrentCoinChanged.AddDynamic(this, &UGameUI::BP_UpdateCoin);
				return true; 
			}
			return false; 
		},
		this,
		BindUpUpdateCoinDelegateHandle,
		0.1);
}

void UGameUI::NativeConstruct()
{
	Super::NativeConstruct();
	BindBpToDelegrate();
}
