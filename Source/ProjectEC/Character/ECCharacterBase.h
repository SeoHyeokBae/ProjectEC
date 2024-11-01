// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EngineMinimal.h"
#include "GameFramework/Character.h"
#include "ECCharacterBase.generated.h"

UENUM()
enum class ECharacterControlType : uint8
{
	Quater,
	None,
};

UCLASS()
class PROJECTEC_API AECCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AECCharacterBase();

protected:
	virtual void SetCharacterControlData(const class UECPlayerControlData* CharacterControlData);

	UPROPERTY(EditAnywhere, Category = CharacterControl, Meta = (AllowPrivateAcess = "true"))
	TMap<ECharacterControlType, class UECPlayerControlData*> CharacterControlManager;
};
