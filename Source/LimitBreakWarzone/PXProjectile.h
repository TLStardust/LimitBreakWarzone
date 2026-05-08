// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"
#include "PXProjectile.generated.h"


USTRUCT(BlueprintType)
struct FGEEffectData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UGameplayEffect> EffectClass;

	// 应用时的层数，默认为 1
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 StackCount = 1;
};


UCLASS()
class LIMITBREAKWARZONE_API APXProjectile : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	APXProjectile();
	
	// 子弹移动组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	class UProjectileMovementComponent* ProjectileMovement;

	// 碰撞球
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	class USphereComponent* SphereComponent;

	// 关键：该子弹命中后要应用给敌人的 Gameplay Effect
	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TSubclassOf<class UGameplayEffect> DamageEffectClass;*/
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS")
	TArray<FGEEffectData> ImpactEffects;

	// 爆炸半径：如果大于 0，则触发范围伤害
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Explosion")
	float ExplosionRadius = 0.0f;

	// 绘制调试球体，方便测试
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS|Debug")
	bool bShowDebugSphere = false;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// 当撞到物体时的回调
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
