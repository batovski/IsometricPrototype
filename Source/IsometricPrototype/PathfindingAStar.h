// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/Vector.h"
#include "DrawDebugHelpers.h"
#include "PathfindingAStar.generated.h"

class APawn;
class USceneComponent;

UCLASS()
class ISOMETRICPROTOTYPE_API APathfindingAStar : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APathfindingAStar();
	~APathfindingAStar();
	TArray<FVector> FindPath(FVector start, FVector end);

	UFUNCTION(BlueprintCallable, Category = "Navigation")
	void RelocateGrid();
protected:

private:
	UPROPERTY(EditAnywhere, Category = "Navigation")
	float CellSize = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Navigation")
	float SpacingX = 174.0;

	UPROPERTY(EditAnywhere, Category = "Navigation")
	float SpacingY = 200.0;

	UPROPERTY(EditAnywhere, Category = "Navigation")
	float Offset = 100.0;

	UPROPERTY(EditAnywhere, Category = "Navigation")
	int SizeX = 6;
	UPROPERTY(EditAnywhere, Category = "Navigation")
	int SizeY = 6;
	int directions = 6;

	APawn* PlayerPawn;
	USceneComponent* Base;
	
	TArray<FVector> directionOdd;
	TArray<FVector> directionEven;
	TArray<TArray<FQuad*>> QuadGrid;

	void CreateGrid();
	void DeleteGrid();
	bool isQuadValid(int x, int y);
	FVector isPosValid(FVector position);
	FQuad* GetQuad(FVector position);
	FQuad* findNodeOnList(TArray<FQuad*> list, FVector position);
	int calculateH(FVector start, FVector end);
	FVector getDelta(FVector start, FVector end);
	void releaseQuads(TArray<FQuad*> set);
	void BeginPlay();

	TArray<FVector> CreatePath(FQuad* current);
	void CheckTheCollision();
	void ResetQuadsInfo();
};

USTRUCT()
struct FQuad
{
	GENERATED_BODY()
	FQuad(bool blocked, int h, int g);
	FQuad();
	FVector position;
	int GridX;
	int GridY;
	bool isBlocked;
	int h;
	int g;

	FQuad* parent;
	
	int getScore();
};