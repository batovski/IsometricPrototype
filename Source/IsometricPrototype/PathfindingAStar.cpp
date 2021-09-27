// Fill out your copyright notice in the Description page of Project Settings.


#include "PathfindingAStar.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "BasicHex.h"
#include "Containers/Array.h"
#include "Components/SceneComponent.h"

// Sets default values
APathfindingAStar::APathfindingAStar()
{
	Base = CreateDefaultSubobject<USceneComponent>(TEXT("Base"));
	SetRootComponent(Base);
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}
FQuad::FQuad(bool _blocked, int _h, int _g)
{
	parent = nullptr;
	position = FVector();
	isBlocked = _blocked;
	h = _h;
	g = _g;
}
FQuad::FQuad()
{
	parent = nullptr;
	position = FVector();
	isBlocked = false;
	h = 0;
	g = 0;
}

APathfindingAStar::~APathfindingAStar()
{
	DeleteGrid();
}
void APathfindingAStar::BeginPlay()
{
	Super::BeginPlay();

	CreateGrid();
}

void APathfindingAStar::CreateGrid()
{
	direction = {
		FVector(-1*SpacingX, 1*Offset, 0), FVector(1 * SpacingX, 1 * Offset ,0),
		FVector(0, 1* SpacingY, 0), FVector(-1 * SpacingX, 1 * Offset, 0),
		FVector(-1 * SpacingX, -1 * Offset, 0), FVector(0, -1* SpacingY, 0)
	};
	QuadGrid.Reserve(SizeX);
	for (int i = 0; i < SizeX; ++i)
	{
		TArray<FQuad*> arr;
		arr.Reserve(SizeY);
		QuadGrid.Add(arr);
		for (int j = 0; j < SizeY; ++j)
		{
			FQuad* newQuad = new FQuad(false, 0, 0);
			QuadGrid[i].Add(newQuad);
		}
	}
}
void APathfindingAStar::RelocateGrid()
{
	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	FVector Position = PlayerPawn->GetActorLocation();

	if (SizeX % 2 == 0)
		SetActorLocation(FVector(Position.X, Position.Y, GetActorLocation().Z) + FVector(-SizeX / 2 * SpacingX, -SizeY / 2 * SpacingY, 0));
	else
		SetActorLocation(FVector(Position.X, Position.Y, GetActorLocation().Z) + FVector(-SizeX / 2 * SpacingX + Offset, -SizeY / 2 * SpacingY, 0));

	for (int i = 0; i < SizeX; ++i)
	{
		for (int j = 0; j < SizeY; ++j)
		{
			if (i % 2 == 0)
				QuadGrid[i][j]->position = GetActorLocation() + FVector(SpacingX * i, SpacingY * j + CellSize/2, 0);
			else
			{
				QuadGrid[i][j]->position = GetActorLocation() + FVector(SpacingX * i, SpacingY * j + Offset+ CellSize / 2, 0);
			}
		}
	}
	CheckTheCollision();
}

void APathfindingAStar::CheckTheCollision()
{
	float RayLength = 80.0;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	for (int i = 0; i < SizeX; ++i)
	{
		for (int j = 0; j < SizeY; ++j)
		{
			FHitResult Hit;
			FVector StartLocation = QuadGrid[i][j]->position;
			FVector EndLocation = StartLocation + (FVector(0,0,-1) * RayLength);
			bool isHit = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionParams);
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, true, -1, 0, 1.f);
			if (isHit)
			{
				AActor* quad = Hit.GetActor();
				if (quad != nullptr)
				{
					ABasicHex* Hex = Cast<ABasicHex>(quad);
					if (Hex)
					{
						QuadGrid[i][j]->position = Hex->GetHexMiddlePoint();
						QuadGrid[i][j]->isBlocked = Hex->isHexBlocked();
						UE_LOG(LogTemp, Warning, TEXT("Satrt %f, %f:"), QuadGrid[i][j]->position.X, QuadGrid[i][j]->position.Y);
					}
				}
			}

		}
	}
}

void APathfindingAStar::DeleteGrid()
{
	//TODO: do i need clean up the meamory?
}

int FQuad::getScore()
{
	return h + g;
}

void APathfindingAStar::ResetQuadsInfo()
{
	for (int i = 0; i < SizeX; ++i)
	{
		for (int j = 0; j < SizeY; ++j)
		{
			QuadGrid[i][j]->g = 0;
			QuadGrid[i][j]->h = 0;
			QuadGrid[i][j]->parent = nullptr;
		}
	}
}

TArray<FVector> APathfindingAStar::FindPath(FVector start, FVector end)
{
	TArray<FVector> empty;
	if (start == end || !isQuadValid(start))
		return empty;

	ResetQuadsInfo();

	FQuad* current = nullptr;

	TArray<FQuad*> openSet, closedSet;
	openSet.Reserve(SizeX * SizeY);
	closedSet.Reserve(SizeX * SizeY);
	auto first = GetQuad(start);

	if (first != nullptr)
	{
		openSet.Push(first);
	}
	while (openSet.Num() != 0)
	{
		int it_index = 0;
		current = *openSet.begin();

		for (int i = 0; i < openSet.Num(); i++) {
			auto node = openSet[i];
			if (node->getScore() <= current->getScore()) {
				current = node;
				it_index = i;
			}
		}

		if (current->position.X == end.X && current->position.Y == end.Y) {
			break;
		}

		closedSet.Push(current);
		openSet.RemoveAt(it_index);

		for (int i = 0; i < directions; ++i) {
			FVector newCoordinates = current->position + direction[i];

			int totalCost = current->g + CellSize;

			FQuad* successor = findNodeOnList(openSet, newCoordinates);
			if (successor == nullptr) {
				successor = GetQuad(newCoordinates);
				if (successor)
				{
					successor->g = totalCost;
					successor->h = calculateH(successor->position, end);
					openSet.Push(successor);
				}
			}
			else if (totalCost < successor->g) {
				successor->parent = current;
				successor->g = totalCost;
				UE_LOG(LogTemp, Warning, TEXT("Called"));
			}
			else if(successor!= nullptr)
				UE_LOG(LogTemp, Warning, TEXT("Called"));
		}
	}
	
	TArray<FVector> path;
	while (current != nullptr) {
		path.Push(current->position);
		current = current->parent;
	}
	releaseQuads(openSet);
	releaseQuads(closedSet);
	return path;
}
bool APathfindingAStar::isQuadValid(FVector position)
{
	
	if (position.X > QuadGrid[SizeX - 1][0]->position.X ||
		position.Y > QuadGrid[0][SizeY - 1]->position.Y ||
		position.X < QuadGrid[0][0]->position.X ||
		position.Y < QuadGrid[0][0]->position.Y)
	{
		return false;
	}
	return true;
}

FQuad* APathfindingAStar::findNodeOnList(TArray<FQuad*> list, FVector position)
{
	for (auto node : list) {
		if (node->position.X == position.X && node->position.Y == position.Y) {
			return node;
		}
	}
	return nullptr;
}
void APathfindingAStar::releaseQuads(TArray<FQuad*> set)
{
	for (int i = 0; i < set.Num(); i++) {
		auto quad = set[i];
		set.RemoveAt(i);
		delete quad;
	}
}

FVector APathfindingAStar::getDelta(FVector start, FVector end)
{
	return{ abs(start.X - end.X),  abs(start.Y - end.Y), 0 };
}

int APathfindingAStar::calculateH(FVector start, FVector end)
{
	return ((double)sqrt(
		(start.X - end.X) * (start.X - end.X)
		+ (start.Y - end.Y) * (start.Y - end.Y)));
}


FQuad* APathfindingAStar::GetQuad(FVector position)
{
	for(int i = 0; i < SizeX; i++)
	{
		for (int j = 0; j < SizeY; j++)
		{
			if (QuadGrid[i][j]->position.X == position.X && QuadGrid[i][j]->position.Y == position.Y)
				return QuadGrid[i][j];
		}
	}
	return nullptr;
}