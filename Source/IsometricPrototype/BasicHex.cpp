// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicHex.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ABasicHex::ABasicHex()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
}

// Called when the game starts or when spawned
void ABasicHex::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABasicHex::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector ABasicHex::GetHexMiddlePoint()
{
	FVector Min;
	FVector Max;

	Mesh->GetLocalBounds(Min, Max);

	return FVector((Max.X + Min.X) / 2, (Max.Y + Min.Y) / 2, Max.Z) + GetActorLocation();
}

