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

 /* 触发逻辑
 *	使用 RAII 来检查所属作用域中是否有移动数据发生了改变
 *	
 *	如果只依赖 RAII 会存在一个问题，因为只有在作用域结束时才会进行变化判断和打印堆栈
 *	如果是以下情况的话则会出现问题
 *
 *	{
 *		MOVEMENT_DEBUG() A1
 *		...
 *		MOVEMENT_DEBUG() A2
 *		DoMove()
 *		{
 *			MOVEMENT_DEBUG() A3
 *			DoMove()
 *		}
 *	}
 *
 *	如果只依赖 RAII，那么打印堆栈的顺序是先打印 A1->A2->A3，再打印 A1->A2，这样会存在歧义
 *	但是比较好的情况应该是先打印 A1->A2，再打印 A1->A2->A3，且 A1 不需要有单独的堆栈信息
 *	因此这里做一些额外的扩展：当 A3 入栈的时候，会更新父节点的数据(A2)，强制触发一次 A2 的检查来判定是否有变化
 *	并且为了防止 A1 被单独触发，当触发 A2 的堆栈信息输出的时候，会标记调用链上的所有节点，跳过后续的检查
 *
 *  目前这一套存在一个问题
 *	如果在 A3 入栈的时候已经触发了 A1->A2，A2 在自己析构的时候如果存在了漏桩的情况（即 A3 之后的代码还有改变速度状态的逻辑，但是没有加桩）
 *	这个时候是无法捕获的！（因为 A2 在被触发了之后就已经加了标记，不需要进一步检查）
 *
 *	{
 *		MOVEMENT_DEBUG() A1
 *		...
 *		MOVEMENT_DEBUG() A2
 *		DoMove()
 *		{
 *			MOVEMENT_DEBUG() A3
 *			DoMove()
 *		}
 *	 	DoMove()			// 这个变化无法被捕获！！！
 *	}
 *
 *	扩展思路：当 A2 被提前触发了,即 A1->A2 的堆栈信息已经输出完毕，那么可以考虑让 A2 的 AfterInfo 覆盖 BeginInfo，然后在 A2 析构的时候再更新并检查一次是否存在 dif
 *	(如果用 AfterInfo 覆盖 BeginInfo 进行拓展，那么就不需要 bIsPrint 来标记了) —— 留给有需要的时候再拓展
 *
 *
 * 	但是换个思考问题的思路：
 * 	如果所有移动操作最终都会经过同一个底层函数，那么只要在这个底层函数开头加桩，就可以大幅降低漏桩概率。
 *	不过这种方式无法覆盖直接手动修改移动数据的情况。
 *
 *	目前功能已经可以覆盖大部分使用场景
 *	只要尽量避免直接手动修改移动参数，并在关键底层移动函数中埋好桩，基本就能定位到状态变化来源。
 *	例如上面的 DoMove 也在函数开头埋了桩 B1，那么输出就是：
 * 	A1->A2->B1
 *	A1->A2->A3->B1
 *	A1->A2->B1
 * 	第三次虽然能捕获变化，但堆栈粒度只精确到 B1 所在的底层函数(发生了漏桩但是依然能捕获，只是堆栈信息需要额外通过上下文进行分析）。
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
