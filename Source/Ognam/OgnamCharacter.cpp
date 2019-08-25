// Fill out your copyright notice in the Description page of Project Settings.

#include "OgnamCharacter.h"
#include "Engine.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "ConstructorHelpers.h"
#include "Animation/AnimBlueprint.h"
#include "UnrealNetwork.h"
#include "OgnamPlayerController.h"

// Sets default values
AOgnamCharacter::AOgnamCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//Create Spring arm and Camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->TargetOffset = FVector(0, 0, 120);
	SpringArm->SetRelativeRotation(FRotator(-30, 0, 0));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;

	//Default mesh
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkMesh(TEXT("SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'"));
	GetMesh()->SetSkeletalMesh(SkMesh.Object);
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));

	//Animations for the mesh, if animation starts to get buggy, check if this code is right.
	static ConstructorHelpers::FObjectFinder<UClass> AnimBP(
		TEXT("/Game/Animation/OgnamCharacterAnimBlueprint.OgnamCharacterAnimBlueprint_C"));
	//static ConstructorHelpers::FObjectFinder<UAnimBlueprint> AnimBP_PIE(
	//	TEXT("AnimBlueprint'/Game/Animation/OgnamCharacterAnimBlueprint.OgnamCharacterAnimBlueprint'"));
	//if (AnimBP_PIE.Object != nullptr)
	//{
	//	GetMesh()->SetAnimInstanceClass(AnimBP_PIE.Object->GeneratedClass);
	//}
	//else
	//{
		GetMesh()->SetAnimInstanceClass(AnimBP.Object);
	//}

	Health = 100.f;
	MaxHealth = 100.f;
	Defense = 0.0f;

	this->bReplicates = true;
	this->GetCharacterMovement()->MaxWalkSpeed = 300;
	bIsAlive = true;
}

void AOgnamCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOgnamCharacter, Health);
	DOREPLIFETIME(AOgnamCharacter, MaxHealth);
	DOREPLIFETIME(AOgnamCharacter, bIsJumping);
	DOREPLIFETIME(AOgnamCharacter, bIsAlive);
	DOREPLIFETIME(AOgnamCharacter, Defense);
}

// Called to bind functionality to input
void AOgnamCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveFoward", this, &AOgnamCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AOgnamCharacter::MoveRight);
	PlayerInputComponent->BindAxis("CameraYaw", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("CameraPitch", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AOgnamCharacter::Jump);
}

void AOgnamCharacter::MoveForward(float Amount)
{
	if (Controller != nullptr && Amount != 0.f)
	{
		AddMovementInput(GetActorForwardVector(), Amount);
	}
}

void AOgnamCharacter::MoveRight(float Amount)
{
	if (Controller != nullptr && Amount != 0.f)
	{
		AddMovementInput(GetActorRightVector(), Amount);
	}
}

float AOgnamCharacter::GetHealth() const
{
	return Health;
}

float AOgnamCharacter::GetMaxHealth() const
{
	return MaxHealth;
}

bool AOgnamCharacter::GetIsJumping() const
{
	return bIsJumping;
}

bool AOgnamCharacter::IsAlive() const
{
	return bIsAlive;
}

void AOgnamCharacter::GetAimHitResult(FHitResult& HitResult, float near, float far)
{
	//shoot ray from camera to see where it should land.
	FVector RayFrom = Camera->GetComponentLocation() + near;
	FVector RayTo = RayFrom + Camera->GetForwardVector() * far;
	FCollisionQueryParams Params(TEXT("cameraPath"), true, this);
	GetWorld()->LineTraceSingleByProfile(HitResult, RayFrom, RayTo, TEXT("BlockAll"), Params);
}

void AOgnamCharacter::ServerJump_Implementation()
{
	bIsJumping = true;
}

float AOgnamCharacter::GetDamageAfterDefense(float Damage)
{
	return Damage * Defense;
}

void AOgnamCharacter::Jump()
{
	ACharacter::Jump();
	bIsJumping = true;
	ServerJump();
}

void AOgnamCharacter::Landed(const FHitResult& FHit)
{
	bIsJumping = false;
}

float AOgnamCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Health -= Damage;
	if (HasAuthority() && Health <= 0)
	{
		Die();
	}
	return Damage;
}

void AOgnamCharacter::Die_Implementation()
{
	GetMesh()->SetCollisionProfileName(TEXT("RagDoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));
	bIsAlive = false;

	//For local player
	AOgnamPlayerController* PlayerController = Cast<AOgnamPlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		return;
	}
	DisableInput(PlayerController);
	PlayerController->OnPawnDeath();
}
