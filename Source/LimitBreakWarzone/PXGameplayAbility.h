// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "PXGameplayAbility.generated.h"
class UInputAction; 

/**
 * 
 */
UCLASS()
class LIMITBREAKWARZONE_API UPXGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPXGameplayAbility();
	
	// 辅助函数：快速获取我们的主角类（替代蓝图里的 Cast To）
	UFUNCTION(BlueprintPure, Category = "Ability Info")
	class ALimitBreakWarzoneCharacter* GetHeroCharacterFromActorInfo() const;
	
	UFUNCTION(BlueprintPure, Category = "Ability|Utility")
	FTransform GetGenericSpawnTransform() const;
	
	UFUNCTION(BlueprintPure, Category = "Ability|Utility")
	void PredictPath(FPredictProjectilePathParams PredictParams, FPredictProjectilePathResult& PredictResult) const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> AbilityIcon; // 技能图标

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UInputAction> AssociatedAction;
	
	// 冷却标签：必须和你在 GE_Cooldown 里设置的 Granted Tag 一致
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FGameplayTag CooldownTag;

	// 激活状态标签：当技能激活时，角色身上会拥有的标签（用于显示黄色高亮）
	// 例如：State.Ability.Aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	FGameplayTag ActiveStatusTag;
	
	UFUNCTION(BlueprintPure, Category = "Ability|Animation")
	float CalculateMontagePlayRate(UAnimMontage* Montage, float TargetDuration) const;
	
};