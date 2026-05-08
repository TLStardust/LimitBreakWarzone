// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PXStatusIconDataAsset.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FStatusIconInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag StatusTag; // 对应的状态标签

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> IconTexture; // 显示的图标
};


UCLASS()
class LIMITBREAKWARZONE_API UPXStatusIconDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TArray<FStatusIconInfo> IconTable;

	// 辅助函数：通过 Tag 找图片
	UFUNCTION(BlueprintPure, Category = "UI")
	UTexture2D* FindIconByTag(FGameplayTag Tag) const;
};
