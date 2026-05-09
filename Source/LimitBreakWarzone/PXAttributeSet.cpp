// Fill out your copyright notice in the Description page of Project Settings.


#include "PXAttributeSet.h"
#include "GameplayEffectExtension.h"

UPXAttributeSet::UPXAttributeSet()
{
	// 初始化数值（以后可以从 DataAsset 读取）
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitAmmo(30.0f);
	InitMaxAmmo(30.0f);
}

void UPXAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// 如果修改的是生命值，确保它不会小于 0，也不会超过最大生命值
	// 相当于蓝图里的 Clamp 节点
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	
	if (Attribute == GetAmmoAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxAmmo());
	}
	
	
}

void UPXAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// 如果接收到了伤害
	if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		const float LocalDamageDone = GetDamage();
		SetDamage(0.f); // 清空临时伤害属性

		if (LocalDamageDone > 0.f)
		{
			// 真正的扣血逻辑
			const float NewHealth = GetHealth() - LocalDamageDone;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
            
			UE_LOG(LogTemp, Warning, TEXT("Character took %f damage! Current Health: %f"), LocalDamageDone, GetHealth());
		}
	}
}