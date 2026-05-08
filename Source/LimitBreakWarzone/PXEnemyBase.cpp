// Fill out your copyright notice in the Description page of Project Settings.


#include "PXEnemyBase.h"
#include "AbilitySystemComponent.h"
#include "PXAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h" // 必须包含这个数学库
#include "Kismet/GameplayStatics.h"   // 用于获取玩家引用


// Sets default values
APXEnemyBase::APXEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UPXAttributeSet>(TEXT("AttributeSet"));

	// 初始化头顶血条
	HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(RootComponent);
	HealthBarWidget->SetRelativeLocation(FVector(0, 0, 100.0f)); // 移到头顶
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::World); 

}

// Called when the game starts or when spawned
void APXEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		// 绑定血量变化监听（这相当于蓝图里的“绑定事件”）
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
			.AddUObject(this, &APXEnemyBase::HealthChanged);
		
		OnHealthChanged.Broadcast(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
	}
	
	
	if (HealthBarWidget)
	{
		// 1. 获取 3D 容器里真正的 UI 实例
		UUserWidget* WidgetInstance = Cast<UUserWidget>(HealthBarWidget->GetUserWidgetObject());
    
		// 2. 将自己 (this) 赋值给 UI 里的变量
		if (WidgetInstance)
		{
			// 这里的 "OwnerEnemy" 必须和你蓝图里定义的变量名完全一致（大小写敏感）
			FProperty* Prop = WidgetInstance->GetClass()->FindPropertyByName(FName("OwnerEnemy"));
			if (Prop)
			{
				void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(WidgetInstance);
				*((APXEnemyBase**)ValuePtr) = this;
			}

			// 3. 握手成功后，立即广播一次，让血条显示初始的 100%
			OnHealthChanged.Broadcast(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
		}
	}
	
	if (AbilitySystemComponent)
	{
		// 监听 GE 增加
		AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &APXEnemyBase::OnActiveGEAdded);
		// 监听 GE 移除
		AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &APXEnemyBase::OnActiveGERemoved);
	}
}

// Called every frame
void APXEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// 3. 实现“始终面向玩家”逻辑
	if (HealthBarWidget && HealthBarWidget->GetUserWidgetObject())
	{
		// 获取玩家摄像机的位置
		FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation();
		
		// 获取血条组件的位置
		FVector WidgetLocation = HealthBarWidget->GetComponentLocation();

		// 计算从血条指向摄像机的旋转（FindLookAtRotation 相当于蓝图里的同名节点）
		FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(WidgetLocation, CameraLocation);

		// 将旋转应用给血条组件
		HealthBarWidget->SetWorldRotation(NewRotation);
	}

}

// Called to bind functionality to input
void APXEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APXEnemyBase::HealthChanged(const FOnAttributeChangeData& Data)
{
	// 当血量变化时，触发委托通知蓝图 UI
	OnHealthChanged.Broadcast(Data.NewValue, AttributeSet->GetMaxHealth());
}

void APXEnemyBase::OnActiveGEAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	// 遍历所有 Tag，寻找以 "State.Effect" 开头的标签
	for (FGameplayTag Tag : AssetTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("State.Effect"))))
		{
			// 绑定层数变化监听
			AbilitySystemComponent->OnGameplayEffectStackChangeDelegate(ActiveHandle)->AddUObject(this, &APXEnemyBase::OnStackChanged);
			
			// 通知 UI 增加了新 Buff
			OnStatusChanged.Broadcast(Tag, SpecApplied.GetStackCount(), false);
		}
	}
}

void APXEnemyBase::OnStackChanged(FActiveGameplayEffectHandle ActiveHandle, int32 NewStack, int32 OldStack)
{
	if (!AbilitySystemComponent) return;

	// 1. 通过 Handle 获取当前的有效效果对象
	const FActiveGameplayEffect* ActiveGE = AbilitySystemComponent->GetActiveGameplayEffect(ActiveHandle);
	if (!ActiveGE) return;

	// 2. 提取该效果携带的所有资产标签 (Asset Tags)
	FGameplayTagContainer AssetTags;
	ActiveGE->Spec.GetAllAssetTags(AssetTags);

	// 3. 寻找状态标签并通知 UI 更新层数
	for (FGameplayTag Tag : AssetTags)
	{
		// 检查标签是否属于 State.Effect 分类
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("State.Effect"))))
		{
			// 通知 UI：标签没变，但层数变了，bRemoved 设为 false
			OnStatusChanged.Broadcast(Tag, NewStack, false);
		}
	}
}

void APXEnemyBase::OnActiveGERemoved(const FActiveGameplayEffect& RemovedEffect)
{
	// 1. 从被移除的效果中提取标签
	FGameplayTagContainer AssetTags;
	RemovedEffect.Spec.GetAllAssetTags(AssetTags);

	for (FGameplayTag Tag : AssetTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("State.Effect"))))
		{
			// 2. 通知 UI：该状态已消失，bRemoved 设为 true
			// 此时 NewStack 传 0 即可
			OnStatusChanged.Broadcast(Tag, 0, true);
		}
	}
}
