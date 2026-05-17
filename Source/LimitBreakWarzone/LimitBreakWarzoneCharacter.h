// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "AbilitySystemInterface.h" // 必须包含接口
#include "GameplayTagContainer.h"
#include "PXAttributeSet.h"
#include "LimitBreakWarzoneCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
struct FOnAttributeChangeData; 

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

// 1. 定义血量变化委托：参数(当前值, 最大值)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerAttributeChangedSignature, float, NewValue, float, MaxValue);
// 2. 定义状态改变委托：参数(标签, 层数, 是否移除)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPlayerStatusChangedSignature, FGameplayTag, StatusTag, int32, NewStack, bool, bRemoved);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFormChangedSignature, UPXFormAsset*, NewForm);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedSignature, float, Ammo, float, MaxAmmo);

UENUM(BlueprintType)
enum class EHeroInputID : uint8
{
	None,
	Confirm,
	Cancel,
	PrimaryAttack,   // 左键
	SecondaryAttack, // 右键
	Skill_E,         // E
	Skill_Shift,     // Shift
	Skill_Q,          // Q (Ultimate)
	Relode			//R
};

UCLASS(config=Game)
class ALimitBreakWarzoneCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
public:
	ALimitBreakWarzoneCharacter();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	
	// 将委托暴露给蓝图
	UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
	FOnPlayerAttributeChangedSignature OnPlayerHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
	FOnPlayerStatusChangedSignature OnPlayerStatusChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
	FOnFormChangedSignature OnFormChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat|UI")
	FOnAmmoChangedSignature OnAmmoChanged;
	
	void OnReloadTagChanged(const FGameplayTag Tag, int32 NewCount);

protected:
	virtual void BeginPlay();
	
	// 3. 定义技能系统组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS")
	class UAbilitySystemComponent* AbilitySystemComponent;

	// 4. 定义三种形态的标签（初学者友好：先写死在这里，以后再重构成 Data Asset）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FGameplayTag CurrentFormTag;
	
	// 关键：存储所有初始技能的数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<class UGameplayAbility>> DefaultAbilities;
	
	// 专门存放不随形态切换的“常驻技能”（如：装弹、跳跃、交互）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|Setup")
	TArray<TSubclassOf<class UGameplayAbility>> GlobalAbilities;
	
	// 当输入被触发时调用的通用函数
	void SendInputToGAS(EHeroInputID InputID, bool bPressed);

	// 辅助函数：赋予初始技能
	void GiveDefaultAbilities();
	
	// 存储火、冰、电三种 DataAsset 的数组
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Setup")
	TArray<TObjectPtr<class UPXFormAsset>> AvailableForms;

	// 当前激活的形态资产引用
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Live")
	TObjectPtr<UPXFormAsset> ActiveFormAsset;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GAS|Attributes")
	TObjectPtr<UPXAttributeSet> AttributeSet;
	
	// 内部使用的监听回调
	void OnHealthChangedNative(const FOnAttributeChangeData& Data);
	void OnAmmoChangedNative(const FOnAttributeChangeData& Data);
    
	// 状态改变相关的监听回调（参考敌人的代码）
	void OnActiveGEAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);
	void OnActiveGERemoved(const FActiveGameplayEffect& RemovedEffect);
	void OnStackChanged(FActiveGameplayEffectHandle ActiveHandle, int32 NewStack, int32 OldStack);
	
	// 通用的输入处理器
	UFUNCTION(BlueprintCallable)
	void AbilityInputPressed(EHeroInputID InputID);
	
	UFUNCTION(BlueprintCallable)
	void AbilityInputReleased(EHeroInputID InputID);
	
	// 假设你有这些具体的 InputAction 变量（如果没有，请在蓝图细节面板赋值）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* Action_LMB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* Action_RMB;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* Action_Q;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* Action_E;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* Action_Shift;

public:
		
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	
	UFUNCTION(BlueprintCallable)
	void SwitchFormLogic(FGameplayTag NewFormTag);
	
	// 在蓝图细节面板指定 WBP_MainHUD
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	// 存储生成的实例
	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;
	
};

