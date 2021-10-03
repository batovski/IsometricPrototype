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
	MovingState = EMovingState::idle;
	CurrentPathIndex = 0;
	NextPointDestance = 60.f;
}

void AIsometricPrototypePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (MovingState == EMovingState::moving)
	{
		if (FVector::Dist2D(PlayerPawn->GetActorLocation(), CurrentPath[CurrentPathIndex]) < NextPointDestance)
		{
			CurrentPathIndex++;
			if (CurrentPathIndex >= CurrentPath.Num())
			{
				CurrentPathIndex = 0;
				CurrentPath.Empty();
				MovingState = EMovingState::idle;
				Pathfinder->RelocateGrid();
			}
			else
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CurrentPath[CurrentPathIndex]);
			}
		}
	}
}
void AIsometricPrototypePlayerController::BeginPlay()
{
	Super::BeginPlay();
	Pathfinder = Cast<APathfindingAStar>(UGameplayStatics::GetActorOfClass(GetWorld(), APathfindingAStar::StaticClass()));
	PlayerPawn = GetPawn();
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
	if (PlayerPawn)
	{
		if (MovingState != EMovingState::moving)
		{
			//float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());
			CurrentPath = Pathfinder->FindPath(PlayerPawn->GetActorLocation(), DestLocation);
			for(int i = 0; i < CurrentPath.Num(); i++)
			{
				UE_LOG(LogTemp, Warning, TEXT("Walk to: (%f,%f)"), CurrentPath[i].X, CurrentPath[i].Y);
			}
			if (CurrentPath.Num() > 0)
			{
				MovingState = EMovingState::moving;
				CurrentPathIndex = 1;
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CurrentPath[CurrentPathIndex]);
			}
		}
	}
}
