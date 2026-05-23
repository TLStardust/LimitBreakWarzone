// Copyright Epic Games, Inc. All Rights Reserved.


#include "LimitBreakWarzonePlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "LimitBreakWarzoneCharacter.h"
#include "Misc/OutputDeviceNull.h"

void ALimitBreakWarzonePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
		UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
	}
	
	// 1. 创建 UI (只创建一次)
	if (IsLocalController() && MainHUDClass)
	{
		MainHUDInstance = CreateWidget<UUserWidget>(this, MainHUDClass);
		if (MainHUDInstance)
		{
			MainHUDInstance->AddToViewport();
			
			// 2. 【关键】下发引用给当前控制的角色
			if (ALimitBreakWarzoneCharacter* MyChar = Cast<ALimitBreakWarzoneCharacter>(GetPawn()))
			{
				MyChar->SetHUDReference(MainHUDInstance);
			}
		}
	}
}

void ALimitBreakWarzonePlayerController::TogglePauseMenu()
{
	// 1. 检查当前是否有效
	if (!MainHUDInstance) return;

	// 2. 获取并反转当前的暂停状态
	bool bIsCurrentlyPaused = UGameplayStatics::IsGamePaused(GetWorld());
	bool bNewPauseState = !bIsCurrentlyPaused;

	// 3. 执行物理暂停（停止世界运行）
	UGameplayStatics::SetGamePaused(GetWorld(), bNewPauseState);

	// 4. 根据新状态切换输入模式
	if (bNewPauseState)
	{
		// 进入菜单态：显示鼠标，允许点击 UI，不响应游戏输入
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(MainHUDInstance->TakeWidget()); // 聚焦到主 UI
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		
		bShowMouseCursor = true;
	}
	else
	{
		// 返回游戏态：隐藏鼠标，锁定视角到视口，响应游戏输入
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		
		bShowMouseCursor = false;
	}

	// 5. 【核心】通知蓝图主 UI：去显示或隐藏你的暂停子页面
	// 我们利用虚幻引擎的反射机制调用蓝图里定义的函数，避免在 C++ 里做复杂的 Cast
	FOutputDeviceNull ar;
	const FString FunctionCall = bNewPauseState ? TEXT("ShowPauseMenu") : TEXT("HidePauseMenu");
	MainHUDInstance->CallFunctionByNameWithArguments(*FunctionCall, ar, NULL, true);
}

void ALimitBreakWarzonePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		// 绑定 ESC 按键
		EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ALimitBreakWarzonePlayerController::TogglePauseMenu);
	}
}

void ALimitBreakWarzonePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 每当控制器控制一个新身体时，就把 UI 引用传过去
	if (ALimitBreakWarzoneCharacter* MyChar = Cast<ALimitBreakWarzoneCharacter>(InPawn))
	{
		if (MainHUDInstance)
		{
			MyChar->SetHUDReference(MainHUDInstance);
		}
	}
}