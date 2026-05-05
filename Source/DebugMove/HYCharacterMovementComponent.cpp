// Fill out your copyright notice in the Description page of Project Settings.


#include "HYCharacterMovementComponent.h"

#include "MovementInfoSubsystem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

namespace HYCharacterMovementComponent
{
	static int32 MovementDebugInfo = 0;
	FAutoConsoleVariableRef CVarCharacterMovementInfo(
		TEXT("a.CharacterMovement.MovementDebugInfo"),
		MovementDebugInfo,
		TEXT("show movement debug info , 0 None , 1 Autonomous , 2 Simulated , 3 Client , 4 Server , 5 All")
	);
}


void UHYCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	MOVEMENT_DEBUG_OBJ(CharacterOwner);
	MOVEMENT_DEBUG_OBJ(CharacterOwner);
	MOVEMENT_DEBUG_OBJ(CharacterOwner);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	{
		MOVEMENT_DEBUG_OBJ(CharacterOwner);
		Velocity = FVector(0.f,0.f,0.f);
	}
	
	MOVEMENT_DEBUG_OBJ(CharacterOwner);
	Velocity = FVector(11.f,1.f,3.f);
	
#if !UE_BUILD_SHIPPING
	ShowMovementInfo();
#endif
}

void UHYCharacterMovementComponent::ShowMovementInfo()
{
	UWorld* World = GetWorld();
	
	if (HYCharacterMovementComponent::MovementDebugInfo == 0 || !CharacterOwner || !World)
	{
		return;
	}
	
	ENetRole LocalRole = CharacterOwner->GetLocalRole();
	if (HYCharacterMovementComponent::MovementDebugInfo == 1 && LocalRole != ROLE_AutonomousProxy)
	{
		return;
	}
	if (HYCharacterMovementComponent::MovementDebugInfo == 2 && LocalRole != ROLE_SimulatedProxy)
	{
		return;
	}
	if (HYCharacterMovementComponent::MovementDebugInfo == 3 && LocalRole == ROLE_Authority)
	{
		return;
	}
	if (HYCharacterMovementComponent::MovementDebugInfo == 4 && LocalRole != ROLE_Authority)
	{
		return;
	}
	
	const FVector CurrentLocation = CharacterOwner->GetActorLocation();
	const FVector CurrentVelocity = Velocity;
	const FVector CurrentAcceleration = Acceleration;
	const float CurrentMaxSpeed = GetMaxSpeed();
	const float CurrentMaxAcceleration = GetMaxAcceleration();
	const FString CurrentMovementMode = UEnum::GetValueAsString(MovementMode);
	const int32 HashIndex = CharacterOwner->GetFName().GetNumber() + LocalRole;
	const FString TextToPlay = FString::Printf(TEXT("[%s] %s : Location %s , Velocity %s , MaxVelocity %f , MovementMode %s , Acceleration %s , MaxAcceleration %f") , 
		*CharacterOwner->GetName(),
		*UEnum::GetValueAsString(LocalRole),
		*CurrentLocation.ToString(),
		*CurrentVelocity.ToString(),
		CurrentMaxSpeed,
		*CurrentMovementMode,
		*CurrentAcceleration.ToString(),
		CurrentMaxAcceleration);
	
	GEngine->AddOnScreenDebugMessage(HashIndex , -1.f , FColor::Black , TextToPlay , false);
	
	UCapsuleComponent *CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	if (CapsuleComponent)
	{
		float CapsuleRadius = CapsuleComponent->GetUnscaledCapsuleRadius();
		float CapsuleHalfHeight = CapsuleComponent->GetUnscaledCapsuleHalfHeight();
		DrawDebugCapsule(World , CurrentLocation , CapsuleHalfHeight , CapsuleRadius , CharacterOwner->GetActorRotation().Quaternion() , FColor::Black);
		
		FVector ArrowStartLocation = CurrentLocation;
		ArrowStartLocation.Z -= CapsuleHalfHeight;
		const static int32 ArrowMaxSize = 50;
		FVector ActorForwardDir = CharacterOwner->GetActorForwardVector();
		const FVector ForwardEnd = ArrowStartLocation + ArrowMaxSize * ActorForwardDir;
		const FVector VelocityDir = ArrowStartLocation + (ArrowMaxSize * (CurrentVelocity.Size() / CurrentMaxSpeed) * CurrentVelocity.GetSafeNormal());
		const FVector AccelerationDir = ArrowStartLocation + (ArrowMaxSize * (CurrentAcceleration.Size() / CurrentMaxAcceleration) * CurrentAcceleration.GetSafeNormal());
		
		
		DrawDebugDirectionalArrow(World , ArrowStartLocation , ForwardEnd , 50.f , FColor::Black);
		DrawDebugDirectionalArrow(World , ArrowStartLocation , VelocityDir , 50.f , FColor::Green);
		DrawDebugDirectionalArrow(World , ArrowStartLocation , AccelerationDir , 50.f , FColor::Red);
	}
}
