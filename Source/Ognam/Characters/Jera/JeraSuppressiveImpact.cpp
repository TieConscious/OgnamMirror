// Fill out your copyright notice in the Description page of Project Settings.

#include "JeraSuppressiveImpact.h"
#include "JeraSuppressiveImpactAction.h"
UJeraSuppressiveImpact::UJeraSuppressiveImpact()
{
	AbilityType = EAbilityType::Unique;
	Cooldown = 4.f;
}

void UJeraSuppressiveImpact::ActivateAbility()
{
	NewObject<UJeraSuppressiveImpactAction>(Target)->RegisterComponent();
}
