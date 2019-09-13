// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability.h"
#include "OgnamCharacter.h"

UAbility::UAbility()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAbility::BeginPlay()
{
	Super::BeginPlay();
	Target = Cast<AOgnamCharacter>(GetOwner());
}

bool UAbility::ShouldShowNumber() const
{
	return false;
}

float UAbility::GetNumber() const
{
	return 0.f;
}

bool UAbility::IsNameStableForNetworking() const
{
	return true;
}

bool UAbility::IsSupportedForNetworking() const
{
	return true;
}

