// Copyright Epic Games, Inc. All Rights Reserved.

#include "LimitBreakWarzoneCharacter.h"
#include "LimitBreakWarzoneProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "AbilitySystemComponent.h"
#include "PXFormAsset.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ALimitBreakWarzoneCharacter

ALimitBreakWarzoneCharacter::ALimitBreakWarzoneCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
	
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	AttributeSet = CreateDefaultSubobject<UPXAttributeSet>(TEXT("AttributeSet"));
	
	PrimaryActorTick.bCanEverTick = false;

}

void ALimitBreakWarzoneCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	GiveDefaultAbilities();
	SwitchFormLogic(FGameplayTag::RequestGameplayTag(FName("State.Form.Fire")));
	
	// 仅在本地控制的玩家机器上创建 UI
	if (IsLocallyControlled() && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(GetWorld(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	
	if (AbilitySystemComponent && AttributeSet)
	{
		// 1. 绑定血量监听
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetHealthAttribute())
			.AddUObject(this, &ALimitBreakWarzoneCharacter::OnHealthChangedNative);

		// 2. 绑定状态监听 (GE 添加与移除)
		AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &ALimitBreakWarzoneCharacter::OnActiveGEAdded);
		AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ALimitBreakWarzoneCharacter::OnActiveGERemoved);

		// 3. 初始广播：进游戏时先喊一遍，让 UI 显示初始值
		OnPlayerHealthChanged.Broadcast(AttributeSet->GetHealth(), AttributeSet->GetMaxHealth());
	}

	
}

//////////////////////////////////////////////////////////////////////////// Input

void ALimitBreakWarzoneCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 确保调用基类逻辑（虽然基类通常为空，但这是好的编程习惯）
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 获取增强输入组件
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// --- 基础移动与视角控制 ---
		
		// 跳跃 (Jumping)
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// 移动 (Moving)
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALimitBreakWarzoneCharacter::Move);

		// 视角 (Looking)
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALimitBreakWarzoneCharacter::Look);


		// --- GAS 技能输入绑定 (重点修改部分) ---

		// 鼠标左键 (LMB / Primary Attack)
		if (Action_LMB)
		{
			EnhancedInputComponent->BindAction(Action_LMB, ETriggerEvent::Started, this, &ALimitBreakWarzoneCharacter::AbilityInputPressed, EHeroInputID::PrimaryAttack);
			EnhancedInputComponent->BindAction(Action_LMB, ETriggerEvent::Completed, this, &ALimitBreakWarzoneCharacter::AbilityInputReleased, EHeroInputID::PrimaryAttack);
		}

		// 鼠标右键 (RMB / Secondary Attack)
		if (Action_RMB)
		{
			EnhancedInputComponent->BindAction(Action_RMB, ETriggerEvent::Started, this, &ALimitBreakWarzoneCharacter::AbilityInputPressed, EHeroInputID::SecondaryAttack);
			EnhancedInputComponent->BindAction(Action_RMB, ETriggerEvent::Completed, this, &ALimitBreakWarzoneCharacter::AbilityInputReleased, EHeroInputID::SecondaryAttack);
		}

		// Q 键 (Skill Q)
		if (Action_Q)
		{
			EnhancedInputComponent->BindAction(Action_Q, ETriggerEvent::Started, this, &ALimitBreakWarzoneCharacter::AbilityInputPressed, EHeroInputID::Skill_Q);
			EnhancedInputComponent->BindAction(Action_Q, ETriggerEvent::Completed, this, &ALimitBreakWarzoneCharacter::AbilityInputReleased, EHeroInputID::Skill_Q);
		}

		// E 键 (Skill E)
		if (Action_E)
		{
			EnhancedInputComponent->BindAction(Action_E, ETriggerEvent::Started, this, &ALimitBreakWarzoneCharacter::AbilityInputPressed, EHeroInputID::Skill_E);
			EnhancedInputComponent->BindAction(Action_E, ETriggerEvent::Completed, this, &ALimitBreakWarzoneCharacter::AbilityInputReleased, EHeroInputID::Skill_E);
		}

		// Shift 键 (Skill Shift)
		if (Action_Shift)
		{
			EnhancedInputComponent->BindAction(Action_Shift, ETriggerEvent::Started, this, &ALimitBreakWarzoneCharacter::AbilityInputPressed, EHeroInputID::Skill_Shift);
			EnhancedInputComponent->BindAction(Action_Shift, ETriggerEvent::Completed, this, &ALimitBreakWarzoneCharacter::AbilityInputReleased, EHeroInputID::Skill_Shift);
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system."), *GetNameSafe(this));
	}
}


void ALimitBreakWarzoneCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ALimitBreakWarzoneCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ALimitBreakWarzoneCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool ALimitBreakWarzoneCharacter::GetHasRifle()
{
	return bHasRifle;
}

void ALimitBreakWarzoneCharacter::SwitchFormLogic(FGameplayTag NewFormTag)
{
	// 逻辑：如果新标签和当前一样，就不切换
	if (CurrentFormTag == NewFormTag) return;

	// 1. 根据传入的 Tag，从数组中找到对应的形态 DataAsset
	UPXFormAsset* FoundForm = nullptr;
	for (UPXFormAsset* Form : AvailableForms)
	{
		if (Form && Form->FormTag == NewFormTag)
		{
			FoundForm = Form;
			break;
		}
	}
	
	// 如果没找到对应的配置，就不进行切换（防止报错）
	if (!FoundForm) return;
	
	// 逻辑：移除旧标签，添加新标签 (类似于蓝图里的 Switch On Gameplay Tag)
	AbilitySystemComponent->RemoveLooseGameplayTag(CurrentFormTag);
	CurrentFormTag = NewFormTag;
	AbilitySystemComponent->AddLooseGameplayTag(CurrentFormTag);
	ActiveFormAsset = FoundForm;
	
	OnFormChanged.Broadcast(FoundForm);
	
}

void ALimitBreakWarzoneCharacter::GiveDefaultAbilities()
{
	// 1. 顶层卫语句：检查 ASC 和 权限
	if (!AbilitySystemComponent || !HasAuthority()) return;

	for (UPXFormAsset* FormAsset : AvailableForms)
	{
		// 2. 循环卫语句：跳过无效的资产
		if (!FormAsset) continue;

		for (const auto& AbilityPair : FormAsset->AbilityMap)
		{
			TSubclassOf<UGameplayAbility> AbilityClass = AbilityPair.Value;
			
			// 3. 循环卫语句：跳过未配置技能的槽位
			if (!AbilityClass) continue;

			// 4. 处理逻辑：映射 Tag 到 InputID
			FGameplayTag InputTag = AbilityPair.Key;
			int32 InputID = static_cast<int32>(EHeroInputID::None);

			if (InputTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.LMB")))) 
				InputID = static_cast<int32>(EHeroInputID::PrimaryAttack);
			
			else if (InputTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.RMB")))) 
				InputID = static_cast<int32>(EHeroInputID::SecondaryAttack);
			
			else if (InputTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Skill_Q")))) 
				InputID = static_cast<int32>(EHeroInputID::Skill_Q);
			
			else if (InputTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Skill_E")))) 
				InputID = static_cast<int32>(EHeroInputID::Skill_E);
			
			else if (InputTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Input.Action.Skill_Shift")))) 
				InputID = static_cast<int32>(EHeroInputID::Skill_Shift);

			// 5. 执行核心功能
			FGameplayAbilitySpec Spec(AbilityClass, 1, InputID, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
}

void ALimitBreakWarzoneCharacter::SendInputToGAS(EHeroInputID InputID, bool bPressed)
{
	if (!AbilitySystemComponent) return;

	// 根据输入 ID 触发技能
	// 这里的逻辑可以进一步优化，目前先留出位置
	if (bPressed)
	{
		// 通知 ASC 输入按下了
		// 注意：这需要你的 GA 设置了对应的 InputID 或者通过 Tag 匹配
		UE_LOG(LogTemp, Log, TEXT("Input Pressed: %d"), static_cast<int32>(InputID));
	}
}

void ALimitBreakWarzoneCharacter::OnHealthChangedNative(const FOnAttributeChangeData& Data)
{
	// 广播给蓝图 UI
	OnPlayerHealthChanged.Broadcast(Data.NewValue, AttributeSet->GetMaxHealth());
}

void ALimitBreakWarzoneCharacter::OnActiveGEAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	// 1. 遍历这个 GE 携带的所有标签
	for (FGameplayTag Tag : AssetTags)
	{
		// 2. 检查是否是我们定义的“状态效果”标签（以 State.Effect 开头）
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("State.Effect"))))
		{
			// 3. 【核心】为这个具体的 GE 实例绑定“层数变化”监听
			// 这样当层数从 1 变 2 时，系统会自动调用 OnStackChanged
			AbilitySystemComponent->OnGameplayEffectStackChangeDelegate(ActiveHandle)->AddUObject(this, &ALimitBreakWarzoneCharacter::OnStackChanged);

			// 4. 广播给 UI：新增了一个状态，层数为初始层数，未移除(false)
			OnPlayerStatusChanged.Broadcast(Tag, SpecApplied.GetStackCount(), false);
		}
	}
}

void ALimitBreakWarzoneCharacter::OnStackChanged(FActiveGameplayEffectHandle ActiveHandle, int32 NewStack, int32 OldStack)
{
	// 1. 找到是哪个 GE 发生了层数变化
	const FActiveGameplayEffect* ActiveGE = AbilitySystemComponent->GetActiveGameplayEffect(ActiveHandle);
	if (!ActiveGE) return;

	FGameplayTagContainer AssetTags;
	ActiveGE->Spec.GetAllAssetTags(AssetTags);

	// 2. 找到对应的状态标签并广播
	for (FGameplayTag Tag : AssetTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("State.Effect"))))
		{
			OnPlayerStatusChanged.Broadcast(Tag, NewStack, false);
		}
	}
}

void ALimitBreakWarzoneCharacter::OnActiveGERemoved(const FActiveGameplayEffect& RemovedEffect)
{
	FGameplayTagContainer AssetTags;
	RemovedEffect.Spec.GetAllAssetTags(AssetTags);

	for (FGameplayTag Tag : AssetTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("State.Effect"))))
		{
			// 1. 广播给 UI：该状态已移除(true)，层数传 0 即可
			OnPlayerStatusChanged.Broadcast(Tag, 0, true);
		}
	}
}

void ALimitBreakWarzoneCharacter::AbilityInputPressed(EHeroInputID InputID)
{
	if (AbilitySystemComponent)
	{
		// 1. 通知 GAS 系统该 ID 对应的按键按下了
		AbilitySystemComponent->AbilityLocalInputPressed(static_cast<int32>(InputID));
		
		// 2. 如果你还保留着 SendGameplayEvent 逻辑，可以在这里继续调用
		// SendInputToGAS(InputID, true);
	}
}

void ALimitBreakWarzoneCharacter::AbilityInputReleased(EHeroInputID InputID)
{
	if (AbilitySystemComponent)
	{
		// 1. 保留原本的信号（给 C++ 逻辑用）
		AbilitySystemComponent->AbilityLocalInputReleased(static_cast<int32>(InputID));

		/* 2. 【核心改进】如果是左键松开，发送一个明确的 Gameplay Event
		if (InputID == EHeroInputID::PrimaryAttack)
		{
			FGameplayTag ReleaseTag = FGameplayTag::RequestGameplayTag(FName("Input.Action.LMB.Released"));
			FGameplayEventData Payload;
			Payload.EventTag = ReleaseTag;
			Payload.Instigator = this; // 告诉系统谁发的
			
			// 传入 Payload 的地址 (&Payload)
			AbilitySystemComponent->HandleGameplayEvent(ReleaseTag, &Payload);
		}*/
	}
}