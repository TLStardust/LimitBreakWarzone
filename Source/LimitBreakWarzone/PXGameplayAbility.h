// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "PXGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class LIMITBREAKWARZONE_API UPXGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPXGameplayAbility();

	// 这个技能所属的形态标签（例如：State.Form.Fire）
	// 我们在 C++ 定义它，所有子类蓝图都能看到
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Info")
	FGameplayTag AbilityFormTag;

	// 辅助函数：快速获取我们的主角类（替代蓝图里的 Cast To）
	UFUNCTION(BlueprintPure, Category = "Ability Info")
	class ALimitBreakWarzoneCharacter* GetHeroCharacterFromActorInfo() const;
	
	UFUNCTION(BlueprintPure, Category = "Ability|Utility")
	FTransform GetGenericSpawnTransform() const;
	
	UFUNCTION(BlueprintPure, Category = "Ability|Utility")
	void PredictPath(FPredictProjectilePathParams PredictParams, FPredictProjectilePathResult& PredictResult) const;
	
};