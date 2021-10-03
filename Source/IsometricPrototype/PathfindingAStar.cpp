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
	GridX = 0;
	GridY = 0;
}
FQuad::FQuad()
{
	parent = nullptr;
	position = FVector();
	isBlocked = false;
	GridX = 0;
	GridY = 0;
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
	directionOdd = {
		FVector(-1, -1, 0), FVector( -1, 0, 0),
		FVector(1, -1, 0), FVector(1, 0, 0),
		FVector(0, 1, 0), FVector(-1, 0, 0)
	};
	directionEven = {
		FVector(-1, 0, 0), FVector(0, -1, 0),
		FVector(1, 0, 0), FVector(1, 1, 0),
		FVector(0, 1, 0), FVector(-1, 1, 0)
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
			newQuad->GridX = i;
			newQuad->GridY = j;
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
				QuadGrid[i][j]->position = GetActorLocation() + FVector(SpacingX * i, SpacingY * j + CellSize / 2, 0);
			else
			{
				QuadGrid[i][j]->position = GetActorLocation() + FVector(SpacingX * i, SpacingY * j + Offset + CellSize / 2, 0);
			}
		}
	}
	CheckTheCollision();
}

void APathfindingAStar::CheckTheCollision()
{
	float RayLength = 240.0;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);

	for (int i = 0; i < SizeX; ++i)
	{
		for (int j = 0; j < SizeY; ++j)
		{
			FHitResult Hit;
			FVector StartLocation = QuadGrid[i][j]->position;
			FVector EndLocation = StartLocation + (FVector(0, 0, -1) * RayLength);
			bool isHit = GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECollisionChannel::ECC_Visibility, CollisionParams);
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, true, 5, 0, 1.f);
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
						UE_LOG(LogTemp, Warning, TEXT("Create: (%f,%f)"), QuadGrid[i][j]->position.X, QuadGrid[i][j]->position.Y);
					}
				}
			}

		}
	}
}

void APathfindingAStar::DeleteGrid()
{
	//TODO: do i need clean up the meamory?
	QuadGrid.Empty();
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
	FVector startPos = isPosValid(start);
	if (end == startPos || startPos == FVector::ZeroVector) {
		return empty;
	}

	ResetQuadsInfo();

	FQuad* current = nullptr;

	TArray<FQuad*> openSet, closedSet;
	openSet.Reserve(SizeX * SizeY);
	closedSet.Reserve(SizeX * SizeY);
	auto first = GetQuad(startPos);

	if (first != nullptr)
	{
		openSet.Push(first);
	}
	while (openSet.Num() != 0)
	{
		int it_index = 0;
		current = openSet[0];

		for (int i = 0; i < openSet.Num(); i++) {
			auto node = openSet[i];
			if (node->getScore() < current->getScore()) {
				current = node;
				it_index = i;
			}
			
			else if(node->getScore() == current->getScore()) {
				if (node->h < current->h)
				{
					current = node;
					it_index = i;
				}
			}
		}

		closedSet.Push(current);
		openSet.RemoveAt(it_index);

		TArray<FVector> direction;
		if (current->GridX % 2 == 0)
			direction = directionOdd;
		else
			direction = directionEven;

		for (int i = 0; i < direction.Num(); i++)
		{
			//Is valid?
			if (isQuadValid(current->GridX + direction[i].X, current->GridY + direction[i].Y))
			{
				FQuad* successor = QuadGrid[current->GridX + direction[i].X][current->GridY + direction[i].Y];
				if (successor->position.X == end.X && successor->position.Y == end.Y)
				{
					successor->parent = current;
					return CreatePath(successor);
				}

				FQuad* quad = findNodeOnList(openSet, successor->position);
				if (quad != nullptr && quad->getScore() < current->getScore())
					continue;

				//Update existing
				else if (quad != nullptr) {
					successor->g = current->g + CellSize;
					successor->h = calculateH(successor->position, end);
					successor->parent = current;
					UE_LOG(LogTemp, Warning, TEXT("Rewrite: (%f,%f)"), successor->position.X, successor->position.Y);
				}
				//Create A new one or Update the exsisting one
				else if (findNodeOnList(closedSet, successor->position) == nullptr 
					|| findNodeOnList(closedSet, successor->position) != nullptr &&
					findNodeOnList(closedSet, successor->position)->getScore() > successor->getScore())
				{
					successor->g = current->g + CellSize;
					successor->h = calculateH(successor->position, end);
					successor->parent = current;
					UE_LOG(LogTemp, Warning, TEXT("Create: (%f,%f)"), successor->position.X, successor->position.Y);
					openSet.Push(successor);
				}
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Could not find the path!"));
	return empty;
}

TArray<FVector> APathfindingAStar::CreatePath(FQuad* current)
{
	TArray<FVector> path;
	while (current != nullptr) {
		path.Insert(current->position,0);
		current = current->parent;
	}
	return path;
}

bool APathfindingAStar::isQuadValid(int x, int y)
{
	
	if (x < 0 || x >= SizeX || y < 0 || y >= SizeY)
	{
		return false;
	}
	if (QuadGrid[x][y]->isBlocked)
		return false;
	return true;
}

FVector APathfindingAStar::isPosValid(FVector position)
{
	for (int i = 0; i < SizeX; i++)
	{
		for (int j = 0; j < SizeY; j++)
		{
			if (QuadGrid[i][j]->position.X == position.X && QuadGrid[i][j]->position.Y == position.Y)
				return QuadGrid[i][j]->position;
			else if (FVector::Dist2D(QuadGrid[i][j]->position, position) < 100.f)
				return QuadGrid[i][j]->position;
		}
	}
	return FVector::ZeroVector;
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
		delete quad;
	}
}

FVector APathfindingAStar::getDelta(FVector start, FVector end)
{
	return{ abs(start.X - end.X),  abs(start.Y - end.Y), 0 };
}

int APathfindingAStar::calculateH(FVector start, FVector end)
{
	auto delta = std::move(getDelta(start, end));
	return static_cast<int>(10 * sqrt(pow(delta.X, 2) + pow(delta.Y, 2)));
}


FQuad* APathfindingAStar::GetQuad(FVector position)
{
	for (int i = 0; i < SizeX; i++)
	{
		for (int j = 0; j < SizeY; j++)
		{
			if (QuadGrid[i][j]->position.X == position.X && QuadGrid[i][j]->position.Y == position.Y)
				return QuadGrid[i][j];
		}
	}
	return nullptr;
}