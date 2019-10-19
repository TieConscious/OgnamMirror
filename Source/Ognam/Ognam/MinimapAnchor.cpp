// Fill out your copyright notice in the Description page of Project Settings.


#include "MinimapAnchor.h"
#include "Components/StaticMeshComponent.h"
#include "ConstructorHelpers.h"

AMinimapAnchor::AMinimapAnchor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("StaticMesh'/Game/Meshes/MinimapAnchor.MinimapAnchor'"));

	AnchorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Anchor Mesh"));
	RootComponent = AnchorMesh;
	AnchorMesh->SetStaticMesh(Mesh.Object);
	//AnchorMesh->bHiddenInGame = true;
	AnchorMesh->SetWorldRotation(FRotator(0.f, 0.f, 90.f));

	Scale = 1.f;
}

void AMinimapAnchor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMinimapAnchor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FVector AMinimapAnchor::FindScaledOffset(AActor* FromActor)
{
	FVector Res = GetActorLocation() - FromActor->GetActorLocation();

	return (Res * Scale);
}


