// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MovementInfoSubsystem.generated.h"


#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)
#define MOVEMENT_DEBUG() FMovementDebuggerItem CONCAT(_FMovementDebuggerItem_ , __LINE__)(this , __FUNCSIG__, __LINE__)
#define MOVEMENT_DEBUG_OBJ(actor) FMovementDebuggerItem CONCAT(_FMovementDebuggerItem_ , __LINE__)(actor , __FUNCSIG__, __LINE__)

struct MovementInfo
{
	FVector Location;
	FRotator Rotation;
	FVector Velocity;
	uint8 MovementMode;
};

/*  换一个思路来满足链式触发
 *	正常来说是需要在 RAII 结束的时候，判断是否发生变化，如果变化了则打印整个堆栈
 *	但是需要打印变化前最近的一个 RAII 对象，更久远但没有发生变化的 RAII 对象就不需要打印了
 *	例如
 *
 *	MOVEMENT_DEBUG() A1
 *	...
 *	MOVEMENT_DEBUG() A2
 *	ChangeFun()
 *	{
 *		MOVEMENT_DEBUG() A3
 *		ChangeFun()
 *	}
 *	
 *	此时应该打印的顺序是先打印 A2 的完整堆栈，再打印 A3 的完整堆栈，但是 A1 不需要有单独的堆栈信息
 *  即: A1->A2 , A1->A2->A3  两条堆栈信息，不需要有 A1 的单独堆栈信息
 *  
 *	有个问题需要思考：
 *	如果在 A3 入栈的时候已经触发了 A1->A2，当 A2 在自己析构的时候如果存在了漏桩的情况（即 A3 之后的代码还有改变速度状态的逻辑，但是没有加桩）
 *	这个时候是无法捕获的！（因为 A2 在被触发了之后就已经加了标记，不需要进一步检查）
 *
 *	扩展思路：当 A2 被提前触发了,即 A1->A2 的堆栈信息已经输出完毕，那么可以考虑让 A2 的 AfterInfo 覆盖 BeginInfo，然后在 A2 析构的时候再更新并检查一次是否存在 dif
 *	(如果用 AfterInfo 覆盖 BeginInfo 进行拓展，那么就不需要 bIsPrint 来标记了) —— 留给有需要的时候再拓展
 */


USTRUCT()
struct FMovementDebuggerItem
{
	GENERATED_BODY()
	FMovementDebuggerItem() = default;
	FMovementDebuggerItem(ACharacter *Target ,const FString &FN , int32 LN);
	~FMovementDebuggerItem();
	
	MovementInfo BeginInfo;
	MovementInfo AfterInfo;
	
	FString FunctionName;
	int32 LineNumber;
	TWeakObjectPtr<ACharacter> TargetActor;
	
	FMovementDebuggerItem* ParentNode = nullptr;
	
	bool bIsPrint = false;
	
	void UpdateInfo(MovementInfo &Target);
	bool CheckInfoIsChange() const;
};


UCLASS()
class DEBUGMOVE_API UMovementInfoSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	static UMovementInfoSubsystem* Get(const UWorld* World);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
public:
	void AddMovementInfo(FMovementDebuggerItem *MovementInfo);
	void PopLastMovementInfo();
	
	void UpdateAndCheckParentInfo(FMovementDebuggerItem *MovementInfo);
	
	void PrintMovementDifInfo(FMovementDebuggerItem *MovementInfo);
	
private:
	TArray<FMovementDebuggerItem *> MovementDebuggerItems;
};
