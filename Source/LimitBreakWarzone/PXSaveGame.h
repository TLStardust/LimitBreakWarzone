// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PXSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class LIMITBREAKWARZONE_API UPXSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	// 存储灵敏度
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
	float Sensitivity = 1.0f;

	
};
