// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/** Main log category used across the project */
DECLARE_LOG_CATEGORY_EXTERN(Logdemo, Log, All);

#if UE_SERVER
#define SV_REPCALL(Var)
#else
#define SV_REPCALL(Var) OnRep_##Var()
#endif

#define ENSURE_AUTH() ensure(HasAuthority())