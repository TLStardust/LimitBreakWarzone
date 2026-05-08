#include "PXBlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"

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