// Fill out your copyright notice in the Description page of Project Settings.


#include "ECCharacterBase.h"
#include "ECPlayerControlData.h"

// Sets default values
AECCharacterBase::AECCharacterBase()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;  //컨트롤러 방향회전
	GetCharacterMovement()->bOrientRotationToMovement = true;		// 이동방향 회전
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 450.0f);
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 250.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	static ConstructorHelpers::FObjectFinder<UECPlayerControlData> QuaterDataRef(TEXT("/Script/ProjectEC.ECPlayerControlData'/Game/ECPlayer/Data/EC_Quater.EC_Quater'"));

	if (QuaterDataRef.Object)
	{
		CharacterControlManager.Add(ECharacterControlType::Quater, QuaterDataRef.Object);
	}
}

void AECCharacterBase::SetCharacterControlData(const UECPlayerControlData* CharacterControlData)
{
	bUseControllerRotationYaw = CharacterControlData->bUseControllerRotationYaw;

	GetCharacterMovement()->bOrientRotationToMovement = CharacterControlData->bOrientRotationToMovement;
	GetCharacterMovement()->bUseControllerDesiredRotation = CharacterControlData->bUseControllerDesiredRotation;
	GetCharacterMovement()->RotationRate = CharacterControlData->RotationRate;
}


