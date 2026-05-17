// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "PXFormAsset.generated.h"

/**
 * 
 */


UCLASS()
class LIMITBREAKWARZONE_API UPXFormAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	// 形态的唯一标签 (例如: State.Form.Fire)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Logic")
	FGameplayTag FormTag;

	// 核心映射：按键标签 -> 对应的技能类
	// 这样我们就把 Input.LMB 和具体的 GA_Fire_LMB 绑定在一起了
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Abilities")
	TMap<FGameplayTag, TSubclassOf<UGameplayAbility>> AbilityMap;

	// 该形态大招的基础冷却时间
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float UltimateBaseCD;

	// 该形态的代表颜色（用于 UI 和材质参数切换）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	FLinearColor FormColor;
	
	// 【核心新增】该形态在主武器栏显示的图标（如：火元素徽标）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> FormIcon;

	// 【核心新增】该形态默认的弹药上限
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float DefaultMaxAmmo = 30.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float ReloadDuration = 1.5f;
	
};
