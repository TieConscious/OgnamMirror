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
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;


	//Create Spring arm and Camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->TargetOffset = FVector(0, 0, 120);
	SpringArm->SetRelativeRotation(FRotator(-30, 0, 0));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bUsePawnControlRotation = true;
	SetActorScale3D(FVector(.75f, .75f, .75f));
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

	BaseMaxHealth = 100.f;
	BaseDefense = 0.0f;
	BaseSpeed = 600.0f;

	MaxHealth = BaseMaxHealth;
	Defense = BaseDefense;
	Speed = BaseSpeed;

	Health = MaxHealth;

	GetCharacterMovement()->MaxAcceleration = 1536.f;
	GetCharacterMovement()->BrakingFrictionFactor = .5f;
	GetCharacterMovement()->AirControl = 2.f;
	GetCharacterMovement()->GravityScale = 1.5f;
	GetCharacterMovement()->JumpZVelocity = 510.f;

	this->bReplicates = true;
	bIsAlive = true;
}

void AOgnamCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOgnamCharacter, Health);
	DOREPLIFETIME(AOgnamCharacter, BaseMaxHealth);
	DOREPLIFETIME(AOgnamCharacter, MaxHealth);
	DOREPLIFETIME(AOgnamCharacter, BaseDefense);
	DOREPLIFETIME(AOgnamCharacter, Defense);
	DOREPLIFETIME(AOgnamCharacter, BaseSpeed);
	DOREPLIFETIME(AOgnamCharacter, Speed);
	DOREPLIFETIME(AOgnamCharacter, bIsJumping);
	DOREPLIFETIME(AOgnamCharacter, bIsAlive);
}

void AOgnamCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	MaxHealth = BaseMaxHealth;
	Defense = BaseDefense;
	Speed = BaseSpeed;

	//Check ending conditions of Modiifers and apply tick.
	for (int i = Modifiers.Num() - 1; i >= 0; i--)
	{
		if (Modifiers[i]->ShouldEnd())
		{
			Modifiers[i]->DestroyComponent();
		}
		else
		{
			Modifiers[i]->TickModifier(DeltaTime);
		}
	}
	GetCharacterMovement()->MaxWalkSpeed = Speed;

	if (NumInputs > 0)
	{
		InputVector = InputVector.GetSafeNormal();
		InputSpeed /= NumInputs;
		AddMovementInput(InputVector, InputSpeed);
	}
	InputVector = FVector::ZeroVector;
	InputSpeed = 0;
	NumInputs = 0;
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

	PlayerInputComponent->BindAction("Basic", IE_Pressed, this, &AOgnamCharacter::BasicPressed);
	PlayerInputComponent->BindAction("Basic", IE_Released, this, &AOgnamCharacter::BasicReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AOgnamCharacter::ReloadPressed);
	PlayerInputComponent->BindAction("Reload", IE_Released, this, &AOgnamCharacter::ReloadReleased);
	PlayerInputComponent->BindAction("Mobility", IE_Pressed, this, &AOgnamCharacter::MobilityPressed);
	PlayerInputComponent->BindAction("Mobility", IE_Released, this, &AOgnamCharacter::MobilityReleased);
	PlayerInputComponent->BindAction("Unique", IE_Pressed, this, &AOgnamCharacter::UniquePressed);
	PlayerInputComponent->BindAction("Unique", IE_Released, this, &AOgnamCharacter::UniqueReleased);
	PlayerInputComponent->BindAction("Utility", IE_Pressed, this, &AOgnamCharacter::UtilityPressed);
	PlayerInputComponent->BindAction("Utility", IE_Released, this, &AOgnamCharacter::UtilityReleased);
	PlayerInputComponent->BindAction("Special", IE_Pressed, this, &AOgnamCharacter::SpecialPressed);
	PlayerInputComponent->BindAction("Special", IE_Released, this, &AOgnamCharacter::SpecialReleased);

}

void AOgnamCharacter::MobilityPressed()
{
	OnMobilityPressed.Broadcast();
}

void AOgnamCharacter::MobilityReleased()
{
	OnMobilityReleased.Broadcast();
}

void AOgnamCharacter::UniquePressed()
{
	OnUniquePressed.Broadcast();
}

void AOgnamCharacter::UniqueReleased()
{
	OnUniqueReleased.Broadcast();
}

void AOgnamCharacter::UtilityPressed()
{
	OnUtilityPressed.Broadcast();
}

void AOgnamCharacter::UtilityReleased()
{
	OnUtilityReleased.Broadcast();
}

void AOgnamCharacter::SpecialPressed()
{
	OnSpecialPressed.Broadcast();
}

void AOgnamCharacter::SpecialReleased()
{
	OnSpecialReleased.Broadcast();
}

void AOgnamCharacter::ReloadPressed()
{
	OnReloadPressed.Broadcast();
}

void AOgnamCharacter::ReloadReleased()
{
	OnReloadReleased.Broadcast();
}

void AOgnamCharacter::BasicPressed()
{
	OnBasicPressed.Broadcast();
}

void AOgnamCharacter::BasicReleased()
{
	OnBasicReleased.Broadcast();
}
 
void AOgnamCharacter::MoveForward(float Amount)
{
	if (Controller != nullptr && Amount != 0.f)
	{
		if (Amount < 0)
		{
			InputVector += GetActorForwardVector() * -1;
		}
		else
		{
			InputVector += GetActorForwardVector();
		}

		InputSpeed += abs(Amount);
		NumInputs++;
	}
}

void AOgnamCharacter::MoveRight(float Amount)
{
	if (Controller != nullptr && Amount != 0.f)
	{
		if (Amount < 0)
		{
			InputVector += GetActorRightVector() * -1;
		}
		else
		{
			InputVector += GetActorRightVector();
		}

		InputSpeed += abs(Amount);
		NumInputs++;
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

UWeapon* AOgnamCharacter::GetWeapon() const
{
	return Weapon;
}

UAbility* AOgnamCharacter::GetUnique() const
{
	return Unique;
}

UAbility* AOgnamCharacter::GetUtility() const
{
	return Utility;
}

UAbility* AOgnamCharacter::GetSpecial() const
{
	return Special;
}

UAbility* AOgnamCharacter::GetMobility() const
{
	return Mobility;
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
	if (HasAuthority())
	{
		Health -= Damage;
		if (Health <= 0)
		{
			Die();
		}
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
