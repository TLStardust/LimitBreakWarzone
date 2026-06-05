// Fill out your copyright notice in the Description page of Project Settings.


#include "PXEnemyBase.h"
#include "AbilitySystemComponent.h"
#include "PXAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h" // 必须包含这个数学库
#include "Kismet/GameplayStatics.h"   // 用于获取玩家引用
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"



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
	HealthBarWidget->SetRelativeLocation(FVector(0, 0, 135.0f)); // 移到头顶
	HealthBarWidget->SetWidgetSpace(EWidgetSpace::World); 
	HealthBarWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	

}

// Called when the game starts or when spawned
void APXEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		// 绑定血量变化监听（这相当于蓝图里的“绑定事件”）
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
			.AddUObject(this, &APXEnemyBase::HealthChanged);
		
		OnHealthChanged.Broadcast(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
		
		// 监听 GE 增加
		AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &APXEnemyBase::OnActiveGEAdded);
		// 监听 GE 移除
		AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &APXEnemyBase::OnActiveGERemoved);
		
		// 构造标签
		// RequestGameplayTag 会去标签库里寻找对应的标签。
		// 注意：如果项目设置里没定义这个标签，它会返回一个无效标签，所以确保已经在编辑器里添加了它。
		FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Character.Faction.Enemy"));

		if (EnemyTag.IsValid())
		{
			AbilitySystemComponent->AddLooseGameplayTag(EnemyTag);
			UE_LOG(LogTemp, Warning, TEXT("!!! SUCCESS: Added Tag %s to %s"), *EnemyTag.ToString(), *GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("!!! ERROR: Tag 'Character.Faction.Enemy' NOT FOUND in settings !!!"));
		}
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
	// 只有血量首次降为 0 时触发
	if (Data.NewValue <= 0.0f && !bIsDead)
	{
		HandleDeath();
	}
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

void APXEnemyBase::HandleDeath()
{
	if (bIsDead) return;
	bIsDead = true;

	// 1. 关闭胶囊体碰撞（防止玩家撞到尸体产生位移冲突）
	// 这相当于蓝图里的 Set Collision Enabled -> No Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 2. 开启布娃娃系统 (Ragdoll)
	// 必须先开启模拟，再设置碰撞预设
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// 3. 停止 AI
	// 让 AI 控制器放弃对这个身体的控制
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
		AIC->UnPossess();
	}
	
	// --- 【新增：销毁 UI】 ---
	if (HealthBarWidget)
	{
		// 1. 先让它不可见，防止销毁瞬间的视觉闪烁
		HealthBarWidget->SetVisibility(false);
		
		// 2. 彻底从 Actor 身上移除该组件并释放内存
		// 这相当于蓝图里的 DestroyComponent 节点
		HealthBarWidget->DestroyComponent();
	}

	// 4. 设置自动销毁时间
	// 5 秒后这具尸体从世界上彻底消失，节省内存
	//SetLifeSpan(5.0f);

	// 5. 触发蓝图事件，让美术去处理火花和音效
	OnEnemyDeath();
}