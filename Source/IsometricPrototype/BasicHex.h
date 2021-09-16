// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "BasicHex.generated.h"

class UStaticMeshComponent;
UCLASS()
class ISOMETRICPROTOTYPE_API ABasicHex : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasicHex();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FVector GetHexMiddlePoint();
private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;
};
