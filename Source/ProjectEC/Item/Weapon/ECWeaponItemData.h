// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/ECItemData.h"
#include "ECWeaponItemData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTEC_API UECWeaponItemData : public UECItemData
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = Weapon)
	TObjectPtr<USkeletalMesh> WeaponMesh;
};
