// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameUI.generated.h"

/**
 * 
 */
UCLASS()
class DEMO_API UGameUI : public UUserWidget
{
	GENERATED_BODY()
protected:
	FTimerHandle BindUpUpdateCoinDelegateHandle;
public:
	virtual void PostNetReceive() override;
	void BindBpToDelegrate();
	
	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintImplementableEvent, Category="UI", meta = (DisplayName = "Update Coin"))
	void BP_UpdateCoin(int32 coin);

	UFUNCTION(BlueprintImplementableEvent, Category="UI", meta = (DisplayName = "Update Health Bar"))
	void BP_UpdateHealthBar(float percent);
	
};
