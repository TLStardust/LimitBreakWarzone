// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PXBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class LIMITBREAKWARZONE_API UPXBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// 获取指定标签对应的所有 GE 堆叠总层数
	UFUNCTION(BlueprintCallable, Category = "GAS|Abilities")
	static int32 GetTotalStackCountWithTag(UAbilitySystemComponent* ASC, FGameplayTag Tag);
	
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	static void DrawPathLine(UObject* WorldContextObject, const TArray<FVector>& Points, FLinearColor Color, float Thickness = 5.0f, float Duration = 0.05f);
	
};
