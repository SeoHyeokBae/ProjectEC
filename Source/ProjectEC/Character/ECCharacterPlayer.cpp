// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECCharacterPlayer.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "../Player/ECPlayerController.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AECCharacterPlayer

AECCharacterPlayer::AECCharacterPlayer()
{
	
	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -100.f), FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetCollisionProfileName("CharacterMesh");

	static::ConstructorHelpers::FObjectFinder<USkeletalMesh> PlayerMeshRef(TEXT("/Script/Engine.SkeletalMesh'/Game/ECPlayer/Mannequin_UE4/Meshes/SK_Mannequin.SK_Mannequin'"));
	if (PlayerMeshRef.Object)
	{
		GetMesh()->SetSkeletalMesh(PlayerMeshRef.Object);
	}

	static::ConstructorHelpers::FClassFinder<UAnimInstance> AnimRef(TEXT("/Script/Engine.AnimBlueprint'/Game/ECPlayer/Blueprint/ABP_ECPlayer.ABP_ECPlayer_C'"));
	if (AnimRef.Class)
	{
		GetMesh()->SetAnimInstanceClass(AnimRef.Class);
	}

	// ют╥б
	{
		static ConstructorHelpers::FObjectFinder<UInputMappingContext>
			InputMappingRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/ECPlayer/Input/IMC_Default.IMC_Default'"));

		if (InputMappingRef.Object)
		{
			DefaultMappingContext = InputMappingRef.Object;
		}
		
		static ConstructorHelpers::FObjectFinder<UInputAction>
			InputMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ECPlayer/Input/Actions/IA_Move.IA_Move'"));

		if (InputMoveRef.Object)
		{
			MoveAction = InputMoveRef.Object;
		}
		
		static ConstructorHelpers::FObjectFinder<UInputAction>
			InputLookRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ECPlayer/Input/Actions/IA_Look.IA_Look'"));

		if (InputLookRef.Object)
		{
			LookAction = InputLookRef.Object;
		}
	}
}

void AECCharacterPlayer::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (AECPlayerController* PlayerController = Cast<AECPlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AECCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AECCharacterPlayer::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AECCharacterPlayer::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AECCharacterPlayer::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.X);
		AddMovementInput(RightDirection, MovementVector.Y);
	}
}

void AECCharacterPlayer::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}