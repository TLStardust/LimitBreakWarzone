// Fill out your copyright notice in the Description page of Project Settings.


#include "PXStatusIconDataAsset.h"

UTexture2D* UPXStatusIconDataAsset::FindIconByTag(FGameplayTag Tag) const
{
	// 1. 开始遍历数组
	for (const FStatusIconInfo& Info : IconTable)
	{
		// 2. 检查标签是否匹配
		// 这里使用 MatchesTagExact 要求完全一致
		// 如果你想让子标签也能匹配父标签（如 State.Effect.Burn.Strong 匹配 State.Effect.Burn），
		// 可以改用 Tag.MatchesTag(Info.StatusTag)
		if (Info.StatusTag == Tag)
		{
			// 3. 找到了，返回对应的图片指针
			return Info.IconTexture;
		}
	}

	// 4. 如果遍历完都没找到，返回空（蓝图里会显示为空图片）
	return nullptr;
}

