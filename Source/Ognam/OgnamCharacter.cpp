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
#include "InteractComponent.h"

// Sets default values
AOgnamCharacter::AOgnamCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//Create Spring arm and Camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->TargetOffset = FVector(0, 0, 90);
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

	this->bReplicates = true;
	this->GetCharacterMovement()->MaxWalkSpeed = 2000;
	bIsAlive = true;
}

// Called when the game starts or when spawned
void AOgnamCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AOgnamCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOgnamCharacter, TeamID);
	DOREPLIFETIME(AOgnamCharacter, Health);
	DOREPLIFETIME(AOgnamCharacter, MaxHealth);
	DOREPLIFETIME(AOgnamCharacter, bIsJumping);
	DOREPLIFETIME(AOgnamCharacter, bIsAlive);
}

// Called every frame
void AOgnamCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckForInteract();
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
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AOgnamCharacter::OgnamCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AOgnamCharacter::OgnamCrouch);
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

void AOgnamCharacter::CheckForInteract()
{
	FHitResult HitResult;
	GetAimHitResult(HitResult, 500);
	bCanInteract = false;
	if (HitResult.bBlockingHit)
	{
		AActor* Actor = HitResult.GetActor();
		UInteractComponent* Interact = Actor->FindComponentByClass<UInteractComponent>();
		if (Interact)
			bCanInteract = true;
	}
}

int AOgnamCharacter::GetTeamID() const
{
	return TeamID;
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

bool AOgnamCharacter::GetIsCrouched() const
{
	return bIsCrouched;
}

bool AOgnamCharacter::IsAlive() const
{
	return bIsAlive;
}

bool AOgnamCharacter::CanInteract() const
{
	return bCanInteract;
}

void AOgnamCharacter::GetAimHitResult(FHitResult& HitResult, float Dist)
{
	//shoot ray from camera to see where it should land.
	//Potentially change this to Hit registeration from screen position
	FVector RayFrom = Camera->GetComponentLocation();
	FVector RayTo = RayFrom + Camera->GetForwardVector() * Dist;
	FCollisionQueryParams Params(TEXT("cameraPath"), true, this);
	GetWorld()->LineTraceSingleByProfile(HitResult, RayFrom, RayTo, TEXT("BlockAll"), Params);
}

void AOgnamCharacter::ServerJump_Implementation()
{
	bIsJumping = true;
}

void AOgnamCharacter::OgnamCrouch()
{
	ACharacter::Crouch(true);
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
	if (Health <= 0 && HasAuthority())
	{
		AOgnamPlayerController* PlayerController = Cast<AOgnamPlayerController>(GetController());
		if (PlayerController != nullptr)
		{
			PlayerController->Die();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Not Ognam Player Controller"));
		}
	}
	return Damage;
}

void AOgnamCharacter::Die_Implementation()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	DisableInput(PlayerController);
	GetMesh()->SetCollisionProfileName(TEXT("RagDoll"));
	GetMesh()->SetSimulatePhysics(true);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));
	bIsAlive = false;
}
