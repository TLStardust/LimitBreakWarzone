// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "PXEnemyBase.generated.h"

// 定义一个多播委托（就像蓝图里的 Event Dispatcher）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeChangedDelegate, float, NewValue, float, MaxValue);
// 当状态改变时通知 UI (增加、移除、层数变动统一通知)
// 参数：Tag, 新层数, 是否已移除
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStatusChanged, FGameplayTag, StatusTag, int32, NewStack, bool, bRemoved);

UCLASS()
class LIMITBREAKWARZONE_API APXEnemyBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APXEnemyBase();
	
	// 实现接口
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	// 暴露给 UI 蓝图的事件：当血量改变时调用
	UPROPERTY(BlueprintAssignable, Category = "GAS|UI")
	FOnAttributeChangedDelegate OnHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|UI")
	FOnStatusChanged OnStatusChanged;
	
	// 监听死亡事件
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnEnemyDeath();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// 防止多次触发死亡逻辑
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsDead = false;

	// 核心死亡处理函数 (C++ 实现)
	virtual void HandleDeath();

	
	// GAS 核心组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	TObjectPtr<class UAbilitySystemComponent > AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "GAS")
	TObjectPtr<class UPXAttributeSet> AttributeSet;

	// 头顶血条组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UWidgetComponent> HealthBarWidget;

	// 当属性发生变化时的回调函数
	void HealthChanged(const FOnAttributeChangeData& Data);
	
	// 在敌人基类里定义图标配置表的引用
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|UI")
	TObjectPtr<class UPXStatusIconDataAsset> StatusIconDataAsset;
	
	// GAS 监听的回调函数
	void OnActiveGEAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);
	void OnActiveGERemoved(const FActiveGameplayEffect& RemovedEffect);
	void OnStackChanged(FActiveGameplayEffectHandle ActiveHandle, int32 NewStack, int32 OldStack);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
