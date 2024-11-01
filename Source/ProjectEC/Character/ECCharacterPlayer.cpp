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

//////////////////////////////////////////////////////////////////////////
// AECCharacterPlayer

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

	// 입력
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
	}

	CurrentCharacterControlType = ECharacterControlType::Quater;
}

void AECCharacterPlayer::BeginPlay()
{
	// Call the base class  
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
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AECCharacterPlayer::Move);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
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

void AECCharacterPlayer::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	//FVector2D MovementVector = Value.Get<FVector2D>();

	//float InputSizeSquared = MovementVector.SquaredLength();
	//float MovementVectorSize = 1.0f;
	//float MovementVectorSizeSquared = MovementVector.SquaredLength();
	//if (MovementVectorSizeSquared > 1.0f)
	//{
	//	MovementVector.Normalize();
	//	MovementVectorSizeSquared = 1.0f;
	//}
	//else
	//{
	//	MovementVectorSize = FMath::Sqrt(MovementVectorSizeSquared);
	//}

	//FVector MoveDirection = FVector(MovementVector.X, MovementVector.Y, 0.f);
	//GetController()->SetControlRotation(FRotationMatrix::MakeFromX(MoveDirection).Rotator());
	//AddMovementInput(MoveDirection, MovementVectorSize);


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