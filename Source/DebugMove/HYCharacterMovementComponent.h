// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HYCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class DEBUGMOVE_API UHYCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	void ShowMovementInfo();
	
	
};
