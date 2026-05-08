// Fill out your copyright notice in the Description page of Project Settings.


#include "PXGameplayAbility.h"
#include "LimitBreakWarzoneCharacter.h" // 包含你的角色头文件
#include "Kismet/GameplayStatics.h"


UPXGameplayAbility::UPXGameplayAbility()
{
	// 默认设置：技能激活时是否自动实例化？
	// InstancedPerActor 代表每个角色拥有一份独立的实例，这对于处理 CD 和变量很安全
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

ALimitBreakWarzoneCharacter* UPXGameplayAbility::GetHeroCharacterFromActorInfo() const
{
	// 从 ActorInfo 中获取 AvatarActor 并转换
	// 这比在蓝图里 GetAvatarActor 再转要高效得多
	return Cast<ALimitBreakWarzoneCharacter>(GetAvatarActorFromActorInfo());
}

FTransform UPXGameplayAbility::GetGenericSpawnTransform() const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar) return FTransform::Identity;

	// 1. 基础位置：Actor 位置
	FVector SpawnLoc = Avatar->GetActorLocation();
	
	// 2. 基础旋转：Actor 旋转
	FRotator SpawnRot = Avatar->GetActorRotation();

	// 3. 进阶逻辑：如果是玩家，使用控制器旋转（准星方向）
	if (APawn* Pawn = Cast<APawn>(Avatar))
	{
		// 这样玩家射击时就是朝准星指的方向，而 AI 射击时是朝它面朝的方向
		SpawnRot = Pawn->GetViewRotation();
	}

	// 4. 向前偏移 100 厘米，向上偏移 50 厘米（大概在胸口高度）
	FVector Offset = SpawnRot.Vector() * 100.0f + FVector(0, 0, 50.0f);
	
	return FTransform(SpawnRot, SpawnLoc + Offset);
}

void UPXGameplayAbility::PredictPath(FPredictProjectilePathParams PredictParams, FPredictProjectilePathResult& PredictResult) const
{
	// 调用引擎底层
	UGameplayStatics::PredictProjectilePath(GetWorld(), PredictParams, PredictResult);
}
