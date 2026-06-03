// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputMappingContext.h"
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
	
	UFUNCTION(BlueprintPure, Category = "Input")
	static FText GetKeyNameForAction(const APlayerController* PC, const UInputAction* Action);
	
	UFUNCTION(BlueprintPure, Category = "GAS|UI")
	static void GetCooldownInfo(UAbilitySystemComponent* ASC, FGameplayTag CooldownTag, float& RemainingTime, float& Duration);
};
