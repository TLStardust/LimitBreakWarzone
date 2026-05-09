// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "PXAttributeSet.generated.h"

/**
 * 
 */

// 这是 GAS 标准宏，用于自动生成 Getter 和 Setter 函数
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


UCLASS()
class LIMITBREAKWARZONE_API UPXAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UPXAttributeSet();

	// 当前生命值
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UPXAttributeSet, Health);

	// 最大生命值
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UPXAttributeSet, MaxHealth);

	// 这是一个“元属性”，它不直接存储数值，而是作为伤害计算的中间桥梁
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UPXAttributeSet, Damage);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Ammo;
	ATTRIBUTE_ACCESSORS(UPXAttributeSet, Ammo);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxAmmo;
	ATTRIBUTE_ACCESSORS(UPXAttributeSet, MaxAmmo);

	// 当属性发生变化前的处理（比如防止生命值超过最大值）
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 当属性发生变化后的处理（比如处理死亡逻辑）
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
