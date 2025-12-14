// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "demo.h"
#include "GameUI.h"
#include "ShooterUI.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AShooterPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	
}

void AShooterPlayerController::OnRep_Pawn()
{
	UE_LOG(LogTemp, Warning, TEXT("AShooterPlayerController::OnRep_Pawn - Role: %s, New Pawn: %s"), *UEnum::GetValueAsString(GetLocalRole()), GetPawn() ? *GetPawn()->GetName() : TEXT("nullptr"));
	APawn* NewPawn = GetPawn();
	if (NewPawn)
	{
		//NewPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);
		//SetupPawnDelegates(NewPawn);
	}
	Super::OnRep_Pawn();
}

void AShooterPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		ShooterCharacter->Tags.Add(PlayerPawnTag);
	}
	if (IsLocalController() && InPawn) 
	{
		UE_LOG(LogTemp, Log, TEXT("SetPawn: Executing Client Bindings. Controller Role: %s"), *UEnum::GetValueAsString(GetLocalRole()));
        
		SetupPawnDelegates(InPawn); 
	}
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(Logdemo, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}

		GameUI = CreateWidget<UGameUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), GameUIClass);
		if (GameUI)
		{
			GameUI->AddToViewport(0);
		}
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Log, TEXT("AShooterPlayerController::OnPossess - Controller: %s, Pawn: %s, Role: %s"), *GetName(), *InPawn->GetName(), *UEnum::GetValueAsString(GetLocalRole()));

	Client_OnPossess(InPawn);
	// subscribe to the pawn's OnDestroyed delegate
}

void AShooterPlayerController::Client_OnPossess_Implementation(APawn* InPawn)
{
	UE_LOG(LogTemp, Log, TEXT("AShooterPlayerController::Client_OnPossess_Implementation , Role: %s"), *UEnum::GetValueAsString(GetLocalRole()));

	//SetupPawnDelegates(InPawn);
}

void AShooterPlayerController::SetupPawnDelegates(APawn* InPawn)
{
	
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		
		ShooterCharacter->OnBulletCountUpdated.RemoveDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.RemoveDynamic(this, &AShooterPlayerController::OnPawnDamaged);
		ShooterCharacter->OnDamageEffect.RemoveDynamic(this, &AShooterPlayerController::OnPawnDamageEffect);
		InPawn->OnDestroyed.RemoveDynamic(this, &AShooterPlayerController::OnPawnDestroyed);
		
		ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);
		ShooterCharacter->OnDamageEffect.AddDynamic(this, &AShooterPlayerController::OnPawnDamageEffect);
 
		InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("AShooterPlayerController::SetupPawnDelegates - Expected a ShooterCharacter"));
	}
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// reset the bullet counter HUD
	GameUI->BP_UpdateBulletCounter(0,0);

	// find the player start
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// select a random player start
		AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

		// spawn a character at the player start
		const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

		if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
		{
			// possess the character
			Possess(RespawnedCharacter);
		}
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	GameUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(GameUI))
	{
		GameUI->BP_UpdateHealthBar(LifePercent);	
	}
}

void AShooterPlayerController::OnPawnDamageEffect()
{
	if (IsValid(GameUI))
	{
		GameUI->BP_PlayDamageEffect();
	}
}
