// Fill out your copyright notice in the Description page of Project Settings.


#include "PXProjectile.h"
#include "AbilitySystemInterface.h" 
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
APXProjectile::APXProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// 2. 创建碰撞球组件并设为根组件 (Root)
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	
	// 设置碰撞半径
	SphereComponent->InitSphereRadius(15.0f);
	
	// 设置碰撞预设 (Projectile 是 UE 预设的子弹碰撞类型)
	SphereComponent->SetCollisionProfileName(TEXT("Projectile"));

	// 【关键】告诉引擎：当这个球体撞击到东西时，调用 OnHit 函数
	// 注意：OnHit 必须带有 UFUNCTION() 宏才能这样绑定
	SphereComponent->OnComponentHit.AddDynamic(this, &APXProjectile::OnHit);

	RootComponent = SphereComponent;

	// 3. 创建子弹移动组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMoveComp"));
	
	// 设置初始速度和最大速度
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	
	// 确保子弹发射方向正确
	ProjectileMovement->bRotationFollowsVelocity = true;
	
	// 设置受重力影响程度 (0.0 代表直线飞行，不往下掉)
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// 4. 设置子弹的寿命 (例如 5秒后自动销毁，防止子弹飞出地图外无限浪费资源)
	InitialLifeSpan = 10.0f;

}

// Called when the game starts or when spawned
void APXProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APXProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APXProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 1. 权限检查：只有服务器能应用 GameplayEffect
	if (!HasAuthority()) 
	{
		Destroy(); // 客户端只负责看烟花（如果有特效），不处理逻辑
		return;
	}

	// 1. 如果有爆炸半径，执行范围逻辑
    if (ExplosionRadius > 0.0f)
    {
        TArray<AActor*> IgnoreActors;
        IgnoreActors.Add(GetOwner()); // 忽略自己（发射者）
        
        TArray<AActor*> OutActors;

        // 核心：球体范围重叠检测
        bool bHadHit = UKismetSystemLibrary::SphereOverlapActors(
            GetWorld(), 
            GetActorLocation(), 
            ExplosionRadius, 
            DetectionObjectTypes,
            nullptr, // 不限制特定类
            IgnoreActors, 
            OutActors
        );

        if (bShowDebugSphere)
        {
            DrawDebugSphere(GetWorld(), GetActorLocation(), ExplosionRadius, 12, FColor::Orange, false, 2.0f);
        }

        // 2. 遍历范围内所有目标并应用 GE 数组
        if (bHadHit)
        {
            for (AActor* OverlappedActor : OutActors)
            {
                UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OverlappedActor);
                UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
                
                if (TargetASC && SourceASC)
                {
                    FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
                    // 这里很重要：即便没有直接撞击到人，我们也把子弹位置作为 HitResult 的起点
                    Context.AddInstigator(GetInstigator(), this);

                	for (const FGEEffectData& EffectData : ImpactEffects)
                	{
                		if (!EffectData.EffectClass) continue;

                		FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectData.EffectClass, 1.0f, Context);
                		if (SpecHandle.IsValid())
                		{
                			// 【核心代码】在这里直接设置 Spec 的层数！
                			// 这样你就不用去改 GE 资产了，代码会动态修改这张“快递单”的层数
                			SpecHandle.Data->SetStackCount(EffectData.StackCount);

                			SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
                		}
                	}
                }
            }
        }
    }
    else
    {
    	if (!OtherActor || OtherActor == GetOwner())
    	{
    		Destroy();
    		return;
    	}

    	// 3. 获取目标和来源的 ASC (使用通用库函数更稳健)
    	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
    	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	
    	if (TargetASC && SourceASC)
    	{
    		// 4. 创建 Context (环境上下文)
    		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
    		Context.AddHitResult(Hit);
    		Context.AddInstigator(GetInstigator(), this);

    		// 5. 创建 Spec (规格句柄)
    		/*FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.0f, Context);
			
			if (SpecHandle.IsValid())
			{
				// 6. 应用效果
				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}*/
		
    		for (const FGEEffectData& EffectData : ImpactEffects)
    		{
    			if (!EffectData.EffectClass) continue;

    			FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(EffectData.EffectClass, 1.0f, Context);
    			if (SpecHandle.IsValid())
    			{
    				// 【核心代码】在这里直接设置 Spec 的层数！
    				// 这样你就不用去改 GE 资产了，代码会动态修改这张“快递单”的层数
    				SpecHandle.Data->SetStackCount(EffectData.StackCount);

    				SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    			}
    		}
    	}

    	// 7. 处理视觉特效 (播放爆炸粒子等)
    	// ExecuteVisualEffects();

    	Destroy();
    }

    // 播放爆炸特效并销毁
    // SpawnExplosionVisuals();
    Destroy();
}