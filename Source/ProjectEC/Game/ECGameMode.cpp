// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECGameMode.h"
#include "../Character/ECCharacterPlayer.h"
#include "UObject/ConstructorHelpers.h"

AECGameMode::AECGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerClassRef(TEXT("/Script/CoreUObject.Class'/Script/ProjectEC.ECCharacterPlayer'"));
	if (PlayerClassRef.Class != NULL)
	{
		DefaultPawnClass = PlayerClassRef.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerClassRef(TEXT("/Script/CoreUObject.Class'/Script/ProjectEC.ECPlayerController'"));
	if (PlayerControllerClassRef.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerClassRef.Class;
	}
}
