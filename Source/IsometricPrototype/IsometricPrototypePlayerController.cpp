// Copyright Epic Games, Inc. All Rights Reserved.

#include "IsometricPrototypePlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IsometricPrototypeCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PathfindingAStar.h"
#include "Engine/World.h"

AIsometricPrototypePlayerController::AIsometricPrototypePlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AIsometricPrototypePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
	}
}
void AIsometricPrototypePlayerController::BeginPlay()
{
	Super::BeginPlay();
	Pathfinder = Cast<APathfindingAStar>(UGameplayStatics::GetActorOfClass(GetWorld(), APathfindingAStar::StaticClass()));
}

void AIsometricPrototypePlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AIsometricPrototypePlayerController::MoveToMouseCursor);

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AIsometricPrototypePlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AIsometricPrototypePlayerController::MoveToTouchLocation);

	InputComponent->BindAction("ResetVR", IE_Pressed, this, &AIsometricPrototypePlayerController::OnResetVR);
}

void AIsometricPrototypePlayerController::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AIsometricPrototypePlayerController::MoveToMouseCursor()
{
	if (AIsometricPrototypeCharacter* MyPawn = Cast<AIsometricPrototypeCharacter>(GetPawn()))
	{
		if (MyPawn->GetCursorToWorld())
		{
			SetNewMoveDestination(MyPawn->GetCursorToWorld()->GetComponentLocation());
		}
	}
}

void AIsometricPrototypePlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void AIsometricPrototypePlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		//float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());
		TArray<FVector> path = Pathfinder->FindPath(MyPawn->GetActorLocation(), DestLocation);
		for(int i = 0; i < path.Num(); i++)
		{
			UE_LOG(LogTemp, Warning, TEXT("INDEX: %d (%f,%f)"),i,path[i].X, path[i].Y);
		}
		// We need to issue move command only if far enough in order for walk animation to play correctly
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);
	}
}
