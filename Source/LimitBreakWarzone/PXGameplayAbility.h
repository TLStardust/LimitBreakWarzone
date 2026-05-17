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
	FText KeyHint; // 按键提示，如 "Q", "E", "Shift"
	
};