// Fill out your copyright notice in the Description page of Project Settings.


#include "MovementInfoSubsystem.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


FMovementDebuggerItem::FMovementDebuggerItem(ACharacter* Target, const FString& FN, int32 LN) : TargetActor(Target) , FunctionName(FN), LineNumber(LN)
{
	UpdateInfo(BeginInfo);
	if (TargetActor.IsValid())
	{
		if (UMovementInfoSubsystem *MovementInfoSubsystem = UMovementInfoSubsystem::Get(TargetActor->GetWorld()))
		{
			MovementInfoSubsystem->AddMovementInfo(this);
		}
	}
}

FMovementDebuggerItem::~FMovementDebuggerItem()
{
	UMovementInfoSubsystem *MovementInfoSubsystem = UMovementInfoSubsystem::Get(TargetActor->GetWorld());
	if (MovementInfoSubsystem)
	{
		UpdateInfo(AfterInfo);
		MovementInfoSubsystem->PopLastMovementInfo(this);
	}
}

void FMovementDebuggerItem::UpdateInfo(MovementInfo& Target)
{
	if (TargetActor.IsValid())
	{
		Target.Location = TargetActor->GetActorLocation();
		Target.Rotation = TargetActor->GetActorRotation();
		Target.Velocity = TargetActor->GetVelocity();
		Target.MovementMode = TargetActor->GetCharacterMovement()->MovementMode;
	}
}

bool FMovementDebuggerItem::CheckInfoIsChange() const
{
	bool bLocationDif = BeginInfo.Location != AfterInfo.Location;
	bool bRotationDif = BeginInfo.Rotation != AfterInfo.Rotation;
	bool bVelocityDif = BeginInfo.Velocity != AfterInfo.Velocity;
	bool bMovementModeDif = BeginInfo.MovementMode != AfterInfo.MovementMode;
	
	return bLocationDif || bRotationDif || bVelocityDif || bMovementModeDif;
}

UMovementInfoSubsystem* UMovementInfoSubsystem::Get(const UWorld* World)
{
	return World ? World->GetSubsystem<UMovementInfoSubsystem>() : nullptr;
}

void UMovementInfoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	MovementDebuggerItems.Reset();
}

void UMovementInfoSubsystem::AddMovementInfo(FMovementDebuggerItem* MovementInfo)
{
	if (MovementDebuggerItems.Num())
	{
		FMovementDebuggerItem* LastItem = MovementDebuggerItems.Last();
		MovementInfo->ParentNode = LastItem;
		UpdateAndCheckParentInfo(MovementInfo);
	}
	MovementDebuggerItems.Add(MovementInfo);
}

void UMovementInfoSubsystem::PopLastMovementInfo(FMovementDebuggerItem *MovementInfo)
{
	if (MovementDebuggerItems.Num() && (MovementInfo == MovementDebuggerItems.Last()))
	{
		FMovementDebuggerItem* LastItem = MovementDebuggerItems.Last();
		PrintMovementDifInfo(LastItem);
		MovementDebuggerItems.Pop();
	}
	else
	{
		// 这里加个警示:堆栈污染了
	}
}

void UMovementInfoSubsystem::UpdateAndCheckParentInfo(FMovementDebuggerItem* MovementInfo)
{
	FMovementDebuggerItem* ParentItem = MovementInfo->ParentNode;
	if (ParentItem)
	{
		ParentItem->UpdateInfo(ParentItem->AfterInfo);
		PrintMovementDifInfo(ParentItem);
	}
}

void UMovementInfoSubsystem::PrintMovementDifInfo(FMovementDebuggerItem* MovementInfo)
{
	if (MovementInfo->TargetActor.IsValid() && !MovementInfo->bIsPrint && MovementInfo->CheckInfoIsChange())
	{
		TArray<FMovementDebuggerItem*> FunctionLinesInfo;
		FMovementDebuggerItem* Item = MovementInfo;
		while (Item)
		{
			FunctionLinesInfo.Add(Item);
			Item->bIsPrint = true;
			Item = Item->ParentNode;
		}
		
		FString FunctionCallStack;
		for (int beg = FunctionLinesInfo.Num() - 1; beg >= 0; --beg)
		{
			FunctionCallStack += FString::Printf(TEXT("%s : %d \n") , *FunctionLinesInfo[beg]->FunctionName , FunctionLinesInfo[beg]->LineNumber);
		}
		
		FunctionCallStack += FString::Printf(TEXT("\nBefore : Location %s , Rotation %s , Velocity %s \n After : Location %s , Rotation %s , Velocity %s \n Dif : Location %s , Rotation %s , Velocity %s \n\n\n"),
			*MovementInfo->BeginInfo.Location.ToString(),
			*MovementInfo->BeginInfo.Rotation.ToString(),
			*MovementInfo->BeginInfo.Velocity.ToString(),
			*MovementInfo->AfterInfo.Location.ToString(),
			*MovementInfo->AfterInfo.Rotation.ToString(),
			*MovementInfo->AfterInfo.Velocity.ToString(),
			*(MovementInfo->AfterInfo.Location - MovementInfo->BeginInfo.Location).ToString(),
			*(MovementInfo->AfterInfo.Rotation - MovementInfo->BeginInfo.Rotation).ToString(),
			*(MovementInfo->AfterInfo.Velocity - MovementInfo->BeginInfo.Velocity).ToString());
		
		UE_VLOG(MovementInfo->TargetActor.Get() , LogTemp , Log , TEXT("%s") , *FunctionCallStack);
	}
};
