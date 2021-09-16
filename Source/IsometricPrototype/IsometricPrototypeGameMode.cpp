// Copyright Epic Games, Inc. All Rights Reserved.

#include "IsometricPrototypeGameMode.h"
#include "IsometricPrototypePlayerController.h"
#include "IsometricPrototypeCharacter.h"
#include "UObject/ConstructorHelpers.h"

AIsometricPrototypeGameMode::AIsometricPrototypeGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AIsometricPrototypePlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDownCPP/Blueprints/TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}