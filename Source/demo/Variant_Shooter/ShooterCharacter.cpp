// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"

#include "demo.h"
#include "DemoPlayerState.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include "Net/UnrealNetwork.h"


AShooterCharacter::AShooterCharacter()
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AShooterCharacter::OnRep_CurrentWeapon(AShooterWeapon* OldWeapon)
{
	OnWeaponDeactivated(OldWeapon);
	OnWeaponActivated(CurrentWeapon);
}

void AShooterCharacter::OnRep_CurrentHP()
{
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterCharacter, CurrentHP);
	DOREPLIFETIME(AShooterCharacter, OwnedWeapons);
	DOREPLIFETIME(AShooterCharacter, CurrentWeapon);
}

void AShooterCharacter::Server_StartFiring_Implementation()
{
	Auth_StartFiring();
}

void AShooterCharacter::Server_StopFiring_Implementation()
{
	Auth_StopFiring();
}

void AShooterCharacter::Server_SwitchWeapon_Implementation()
{
	Auth_SwitchWeapon();
}

void AShooterCharacter::Auth_StopFiring()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (CurrentWeapon)
	{
		CurrentWeapon->Auth_StopFiring();
	}
}

void AShooterCharacter::Auth_StartFiring()
{
	if (GetLocalRole() != ROLE_Authority)
		return;

	if (CurrentWeapon)
	{
		CurrentWeapon->Auth_StartFiring();
	}
}

void AShooterCharacter::Auth_SwitchWeapon()
{
	if (OwnedWeapons.Num() > 1)
	{
		CurrentWeapon->DeactivateWeapon();
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			WeaponIndex = 0;
		}
		else {
			++WeaponIndex;
		}
		CurrentWeapon = OwnedWeapons[WeaponIndex];
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// reset HP to max
	CurrentHP = MaxHP;

	// update the HUD
	OnDamaged.Broadcast(1.0f);
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);
	}

}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CurrentHP <= 0.0f)
		return 0.0f;

	Auth_TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	Local_PlayHitImpactFX();

	return Damage;
}

float AShooterCharacter::Auth_TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (GetLocalRole() != ROLE_Authority)
		return 0.0f;
	
	CurrentHP -= Damage;
	
	SV_REPCALL(CurrentHP);
	
	if (CurrentHP <= 0.0f)
	{
		Auth_Die(EventInstigator);
	}
	return Damage;
}

void AShooterCharacter::Local_PlayHitImpactFX()
{
	if (!IsLocallyControlled())
		return;
    UE_LOG(LogTemp, Log, TEXT("%s::Local_PlayHitImpactFX - Role: %s"), *GetName(), *UEnum::GetValueAsString(GetLocalRole()));
	OnDamageEffect.Broadcast();
}

void AShooterCharacter::DoStartFiring()
{
	if (HasAuthority())
	{
		Auth_StartFiring();
	}
	else
	{
		Server_StartFiring();
	}
}

void AShooterCharacter::DoStopFiring()
{
	if (HasAuthority())
	{
		Auth_StopFiring();
	}
	else
	{
		Server_StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	if (HasAuthority())
	{
		Auth_SwitchWeapon();
	}
	else
	{
		Server_SwitchWeapon();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// spawn the new weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// add the weapon to the owned list
			OwnedWeapons.Add(AddedWeapon);

			// if we have an existing weapon, deactivate it
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// switch to the new weapon
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	UE_LOG(LogTemp, Log, TEXT("%s::OnWeaponActivated - Role: %s"), *GetName(), *UEnum::GetValueAsString(GetLocalRole()));
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());
	
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// unused
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;

}

void AShooterCharacter::Auth_Die(AController* KillerController)
{
	if (KillerController && KillerController != GetController())
	{
		if (ADemoPlayerState* KillerPS = Cast<ADemoPlayerState>(KillerController->PlayerState))
		{
			int32 KillReward = 500; 
			KillerPS->Auth_AddCoins(KillReward);
		}
	}	
	
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::Auth_OnRespawn, RespawnTime, false);
	MC_Die();
}

void AShooterCharacter::MC_Die_Implementation()
{
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}
	
	GetCharacterMovement()->StopMovementImmediately();
	DisableInput(nullptr);
	
	BP_OnDeath();
	if (IsLocallyControlled())
	{
		OnBulletCountUpdated.Broadcast(0, 0);
	}
}

void AShooterCharacter::Auth_OnRespawn()
{
	AController* SavedController = GetController();

	Destroy();

	if (SavedController)
	{
		if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->RestartPlayer(SavedController); 
		}
	}
}
