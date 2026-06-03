#include "PXBlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

int32 UPXBlueprintFunctionLibrary::GetTotalStackCountWithTag(UAbilitySystemComponent* ASC, FGameplayTag Tag)
{
	if (!ASC) return 0;
	
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(Tag));
	
	TArray<FActiveGameplayEffectHandle> ActiveGEHandles = ASC->GetActiveEffects(Query);

	int32 TotalStacks = 0;
	for (const FActiveGameplayEffectHandle& Handle : ActiveGEHandles)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		if (ActiveGE)
		{
			TotalStacks += ActiveGE->Spec.GetStackCount();
		}
	}

	return TotalStacks;
}

void UPXBlueprintFunctionLibrary::DrawPathLine(UObject* WorldContextObject, const TArray<FVector>& Points, FLinearColor Color, float Thickness, float Duration)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World || Points.Num() < 2) return;

	// 循环数组：从 0 连到 1，从 1 连到 2...
	for (int32 i = 0; i < Points.Num() - 1; ++i)
	{
		// 直接调用引擎底层的画线函数
		DrawDebugLine(
			World,
			Points[i],      // 起点
			Points[i+1],    // 终点
			Color.ToFColor(true),
			false,          // 是否持久显示（我们选 false，因为每帧都在重画）
			Duration,       // 线的寿命
			0,              // 深度优先级
			Thickness       // 线的粗细
		);
	}
}

FText UPXBlueprintFunctionLibrary::GetKeyNameForAction(const APlayerController* PC, const UInputAction* Action)
{
	if (!PC || !Action) return FText::GetEmpty();

	// 1. 获取增强输入的本地子系统
	const ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer) return FText::GetEmpty();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return FText::GetEmpty();

	// 2. 查询该 Action 映射的所有按键
	TArray<FKey> MappedKeys = Subsystem->QueryKeysMappedToAction(Action);

	// 3. 返回第一个按键的名称（例如 "Q" 或 "Left Mouse Button"）
	if (MappedKeys.Num() > 0)
	{
		FKey TargetKey = MappedKeys[0];

		// --- 核心：简写字典逻辑 ---
		
		// 鼠标左键
		if (TargetKey == EKeys::LeftMouseButton) return FText::FromString("LMB");
		
		// 鼠标右键
		if (TargetKey == EKeys::RightMouseButton) return FText::FromString("RMB");
		
		// 鼠标中键
		if (TargetKey == EKeys::MiddleMouseButton) return FText::FromString("MMB");

		// 左 Shift 键 (如果你觉得 "Left Shift" 太长)
		if (TargetKey == EKeys::LeftShift) return FText::FromString("Shift");

		// 左 Ctrl 键
		if (TargetKey == EKeys::LeftControl) return FText::FromString("Ctrl");

		// 空格键
		if (TargetKey == EKeys::SpaceBar) return FText::FromString("Space");

		// --- 如果不在字典里，则返回原本的名称 ---
		return TargetKey.GetDisplayName();
	}

	return FText::FromString("None");
}

void UPXBlueprintFunctionLibrary::GetCooldownInfo(UAbilitySystemComponent* ASC, FGameplayTag CooldownTag, float& RemainingTime, float& Duration)
{
	RemainingTime = 0.f;
	Duration = 0.f;

	if (!ASC || !CooldownTag.IsValid()) return;

	// 1. 创建查询
	FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(FGameplayTagContainer(CooldownTag));
	
	// 2. 使用正确的类型接收：TArray<TPair<float, float>>
	// TPair 是 C++ 中的键值对，Key 存剩余时间，Value 存总时长
	TArray<TPair<float, float>> TimePairs = ASC->GetActiveEffectsTimeRemainingAndDuration(Query);

	if (TimePairs.Num() > 0)
	{
		// 3. 提取第一个匹配到的冷却效果
		// Key 对应 RemainingTime, Value 对应 Duration
		RemainingTime = TimePairs[0].Key;
		Duration = TimePairs[0].Value;
	}
}