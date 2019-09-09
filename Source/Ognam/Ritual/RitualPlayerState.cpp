// Fill out your copyright notice in the Description page of Project Settings.


#include "RitualPlayerState.h"
#include "RitualGameState.h"
#include "Ognam/OgnamShooter.h"
#include "Ognam/OgnamCharacter.h"
#include "Engine/World.h"
#include "UnrealNetwork.h"
#include "Characters/Hereira/Hereira.h"

ARitualPlayerState::ARitualPlayerState()
{
	Team = TEXT("Undefined");
	bIsAlive = false;
	PawnClass = AHereira::StaticClass();
}

void ARitualPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARitualPlayerState, Team);
	DOREPLIFETIME(ARitualPlayerState, PawnClass);
}

void ARitualPlayerState::SetTeamIndex(int32 index)
{
	TeamIndex = index;
}

int32 ARitualPlayerState::GetTeamIndex() const
{
	return TeamIndex;
}

void ARitualPlayerState::SetIsAlive(bool Value)
{
	bIsAlive = Value;
}

bool ARitualPlayerState::IsAlive() const
{
	return bIsAlive;
}

UClass* ARitualPlayerState::GetPawnClass()
{
	return PawnClass;
}

void ARitualPlayerState::SetPawnClass(UClass* Pawn)
{
	PawnClass = Pawn;
}

FName ARitualPlayerState::GetSide() const
{
	ARitualGameState* GameState = GetWorld()->GetGameState<ARitualGameState>();
	if (GameState == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s Game state not valid!"), __FUNCTION__);
		return FName();
	}

	if (Team == GameState->GetCurrentOffenseTeam())
	{
		return GameState->OffenseName;
	}
	else if (Team == GameState->GetCurrentDefenseTeam())
	{
		return GameState->DefenseName;
	}
	UE_LOG(LogTemp, Warning, TEXT("%s Team name not valid!"), __FUNCTION__);
	return FName();
}