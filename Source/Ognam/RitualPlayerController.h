// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OgnamPlayerController.h"
#include "RitualPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class OGNAM_API ARitualPlayerController : public AOgnamPlayerController
{
	GENERATED_BODY()

public:
	ARitualPlayerController();

	/*
	**	Binded Function
	*/
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void OnPawnDeath() override;
	virtual void Tick(float DeltaTime) override;

	/*
	**	Getters, Setters
	*/
	UFUNCTION(BlueprintCallable)
	float GetInteractionProgress() const;

	/*
	**	Exported Function
	*/
	UFUNCTION(BlueprintCallable)
	void ShowCharacterSelection();

	UFUNCTION(BlueprintCallable)
	void HideCharacterSelection();

	UFUNCTION(BlueprintCallable)
	bool CanInteract() const;

	UFUNCTION(Server, WithValidation, Unreliable)
	void ServerStartInteract();
	bool ServerStartInteract_Validate() { return true; };
	void ServerStartInteract_Implementation();

	UFUNCTION(Server, WithValidation, Unreliable)
	void ServerStopInteract();
	bool ServerStopInteract_Validate() { return true; };
	void ServerStopInteract_Implementation();

	UFUNCTION(Client, Reliable)
	void StartInteract();
	void StartInteract_Implementation();

	UFUNCTION(Client, Reliable)
	void StopInteract();
	void StopInteract_Implementation();

protected:
	/*
	**	Internal Function
	*/
	void ToggleChangeCharacterUI();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateInteractionTime(float DeltaTime);

	void CheckInteractionCompletion();

	class IInteractable* GetTargetedInteractable() const;

	UFUNCTION(Server, WithValidation, Reliable)
	void ServerChangeCharacter(UClass* CharacterClass);
	bool ServerChangeCharacter_Validate(UClass* CharacterClass) { return true; };
	void ServerChangeCharacter_Implementation(UClass* CharacterClass);

	/*
	**	Props
	*/
	UPROPERTY(Replicated)
	bool bInteracting;

	UPROPERTY(Replicated)
	float InteractionTime;

	IInteractable* TargetedInteractable;

	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<class UUserWidget> CharacterSelectionHUDClass;

	UPROPERTY(EditAnywhere, Category = "Widgets")
	class UUserWidget* CharacterSelectionHUD;

	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<class UUserWidget> InteractionBarClass;

	UPROPERTY(EditAnywhere, Category = "Widgets")
	class UUserWidget* InteractionBar;

};
