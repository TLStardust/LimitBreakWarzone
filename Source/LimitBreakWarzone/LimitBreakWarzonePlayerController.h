// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LimitBreakWarzonePlayerController.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class LIMITBREAKWARZONE_API ALimitBreakWarzonePlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> PauseMenuInstance;

	bool bIsPaused = false;

	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputAction> PauseAction;

	virtual void SetupInputComponent() override;
	
	virtual void OnPossess(APawn* InPawn) override;
public:
	// 切换暂停菜单
	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();
	
	
	
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> MainHUDClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> MainHUDInstance;


	// End Actor interface
};
