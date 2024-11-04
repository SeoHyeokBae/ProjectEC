// Copyright Epic Games, Inc. All Rights Reserved.

#include "ECCharacterPlayer.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "../Player/ECPlayerController.h"
#include "ECPlayerControlData.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

AECCharacterPlayer::AECCharacterPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 800.0f; 
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	CameraBoom->SetRelativeRotation(FRotator(-50.f, 0.f, 0.f));

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

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

#pragma region InputAction
	{
		static ConstructorHelpers::FObjectFinder<UInputMappingContext>
			InputMappingRef(TEXT("/Script/EnhancedInput.InputMappingContext'/Game/ECPlayer/Input/IMC_Quater.IMC_Quater'"));

		if (InputMappingRef.Object)
		{
			DefaultMappingContext = InputMappingRef.Object;
		}
		
		static ConstructorHelpers::FObjectFinder<UInputAction>
			InputMoveRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ECPlayer/Input/Actions/IA_Quater_Move.IA_Quater_Move'"));

		if (InputMoveRef.Object)
		{
			MoveAction = InputMoveRef.Object;
		}

		static ConstructorHelpers::FObjectFinder<UInputAction>
			InputRunRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ECPlayer/Input/Actions/IA_Run.IA_Run'"));

		if (InputRunRef.Object)
		{
			RunAction = InputRunRef.Object;
		}

		static ConstructorHelpers::FObjectFinder<UInputAction>
			InputAttackRef(TEXT("/Script/EnhancedInput.InputAction'/Game/ECPlayer/Input/Actions/IA_Attack.IA_Attack'"));

		if (InputAttackRef.Object)
		{
			AttackAction = InputAttackRef.Object;
		}
	}
#pragma endregion

	CurrentCharacterControlType = ECharacterControlType::Quater;
}

void AECCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	SetCharacterControl(CurrentCharacterControlType);
}

void AECCharacterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLookController();
}

void AECCharacterPlayer::UpdateLookController()
{
	// 마우스 위치 가져오기
	FHitResult HitResult;
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController && PlayerController->GetHitResultUnderCursor(ECC_Visibility, false, HitResult))
	{
		FVector AimDirection = HitResult.ImpactPoint - GetActorLocation();
		AimDirection.Normalize();

		// 캐릭터 회전
		FRotator AimRot = AimDirection.Rotation();
		PlayerController->SetControlRotation(AimRot);
	}
}

void AECCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AECCharacterPlayer::Move);
		// Run
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Started, this, &AECCharacterPlayer::Run);
		EnhancedInputComponent->BindAction(RunAction, ETriggerEvent::Completed, this, &AECCharacterPlayer::RunStop);
		// Attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &AECCharacterPlayer::Attack);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AECCharacterPlayer::AttackStop);
	}
}

void AECCharacterPlayer::ChangeCharacterControl()
{
	if (CurrentCharacterControlType == ECharacterControlType::Quater)
		SetCharacterControl(ECharacterControlType::Quater);
}

void AECCharacterPlayer::SetCharacterControl(ECharacterControlType NewControlType)
{
	UECPlayerControlData* NewCharacterControl = CharacterControlManager[NewControlType];
	check(NewCharacterControl);

	SetCharacterControlData(NewCharacterControl);

	// Add Input Mapping Context
	if (AECPlayerController* PlayerController = Cast<AECPlayerController>(GetController()))
	{
		// 마우스 화면 가두기
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		PlayerController->SetInputMode(InputMode);

		PlayerController->bShowMouseCursor = true;

		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();

			UInputMappingContext* NewMappingContext = NewCharacterControl->InputMappingContext;
			if(NewMappingContext)
				Subsystem->AddMappingContext(NewMappingContext, 0);
		}
	}
}

void AECCharacterPlayer::SetCharacterControlData(const UECPlayerControlData* CharacterControlData)
{
	Super::SetCharacterControlData(CharacterControlData);

	// 컨트롤 데이터 에셋 적용
	CameraBoom->SetRelativeRotation(CharacterControlData->RelativeRotation);
	CameraBoom->TargetArmLength			= CharacterControlData->TargetArmLength;
	CameraBoom->bUsePawnControlRotation = CharacterControlData->bUsePawnControlRotation;
	CameraBoom->bInheritPitch			= CharacterControlData->bInheritPitch;
	CameraBoom->bInheritYaw				= CharacterControlData->bInheritYaw;
	CameraBoom->bInheritRoll			= CharacterControlData->bInheritRoll;
	CameraBoom->bDoCollisionTest		= CharacterControlData->bDoCollisionTest;
}

#pragma region InputFunction
void AECCharacterPlayer::Move(const FInputActionValue& Value)
{
	FVector	Axis = Value.Get<FVector>();
	AddMovementInput(Axis.XAxisVector, Axis.X);
	AddMovementInput(Axis.YAxisVector, Axis.Y);
	//mPlayerController->SetControlRotation(mAimRot);
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

void AECCharacterPlayer::Run(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

void AECCharacterPlayer::RunStop(const FInputActionValue& Value)
{
	GetCharacterMovement()->MaxWalkSpeed = 250.f;
}

void AECCharacterPlayer::Attack(const FInputActionValue& Value)
{
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	//GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Blue, TEXT("Fire"));
}

void AECCharacterPlayer::AttackStop(const FInputActionValue& Value)
{
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}
#pragma endregion
