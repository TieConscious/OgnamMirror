// Fill out your copyright notice in the Description page of Project Settings.

#include "Maxwell.h"
#include "MaxwellSniperRifle.h"
#include "MaxwellShadowShift.h"
#include "MaxwellVault.h"
//#include "MaxwellClaretStrike.h"

AMaxwell::AMaxwell()
{
	CharacterName = FText::FromString("Maxwell");

	BaseMaxHealth = 160.f;
	MaxHealth = BaseMaxHealth;
	Health = BaseMaxHealth;
}

void AMaxwell::BeginPlay()
{
	Super::BeginPlay();
	Weapon = NewObject<UMaxwellSniperRifle>(this, TEXT("Weapon"));
	Mobility = NewObject<UMaxwellShadowShift>(this, TEXT("Mobility"));
	Unique = NewObject<UMaxwellVault>(this, TEXT("Unique"));

	Weapon->RegisterComponent();
	Mobility->RegisterComponent();
	Unique->RegisterComponent();
}
