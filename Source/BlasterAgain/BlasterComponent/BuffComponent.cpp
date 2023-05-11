// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "BlasterAgain/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}



void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	HealRampUp(DeltaTime);
	ChargeRampUp(DeltaTime);
}

void UBuffComponent::JumpUp(float LastJumpVelocity,float JumpVelocity, float JumpupTime)
{
	if (Character == nullptr) return;

	Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	InitialJumpZVelocity = LastJumpVelocity;
	Character->GetWorldTimerManager().SetTimer(
		SpeedUpTimer,
		this,
		&UBuffComponent::ResetJump,
		JumpupTime
	);
	MulticastJumpBuff(JumpVelocity);
}

void UBuffComponent::ResetJump()
{
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpZVelocity;
	}
	MulticastJumpBuff(InitialJumpZVelocity);
}



void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}


void UBuffComponent::SpeedUp(float DefaultWalkSpeed,float DefaultCrouchSpeed, float AcclerateSpeedWalk, float AcclerateCrouch, float SpeedTime)
{
	if (Character == nullptr) return;

	Character->GetCharacterMovement()->MaxWalkSpeed = AcclerateSpeedWalk;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = AcclerateCrouch;

	LastWalkSpeed = DefaultWalkSpeed;
	LastCrouchSpeed = DefaultCrouchSpeed;
	Character->GetWorldTimerManager().SetTimer(
		SpeedUpTimer,
		this,
		&UBuffComponent::SpeedDown,
		SpeedTime
	);
	MulticastSpeedBuff(AcclerateSpeedWalk, AcclerateCrouch);//多播一下让其他客户端也知道自己的速度提升了
}

void UBuffComponent::SpeedDown() 
{
	Character->GetCharacterMovement()->MaxWalkSpeed = LastWalkSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = LastCrouchSpeed;
	MulticastSpeedBuff(LastWalkSpeed, LastCrouchSpeed);//时间到了，多播一下降回来

}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}


void UBuffComponent::Heal(float HealAmount, float HealingTime)//这个函数只是用来设置参数的，真正启动治疗的是tickcomponentt调用healrampup
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsElimed()) return;//要是角色不存在或者已经被淘汰了就不治疗了

	const float HealThisFrame = HealingRate * DeltaTime;//每一帧的回血量
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));//设置血量
	Character->UpdateHUDHealth();

	HealedAmount += HealThisFrame * AmountToHeal;//回血量的计数
	if (Character->IsElimed() || Character->GetHealth() == Character->GetMaxHealth() || HealedAmount == AmountToHeal) //如果治疗过程中玩家被淘汰了或者血量恢复到了最大值或者血量已经恢复到了最大的治疗效果但是还没回满都停止治疗
	{
		HealedAmount = 0;
		AmountToHeal = 0;
		bHealing = false;
	}
}

void UBuffComponent::RechargeShiled(float RechargeAmount, float RechargingTime ,bool bSlowly)
{
	bRecharge = true;
	bSlowlyCharged = bSlowly;
	ChargingShiledRate = RechargeAmount / RechargingTime;
	AmountToRechargeShiled += RechargeAmount;
	FString Message2 = FString::SanitizeFloat(AmountToRechargeShiled);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			60,
			FColor::Red,
			Message2
		);
	}
}

void UBuffComponent::ChargeRampUp(float DeltaTime)
{
	if (!bRecharge || Character == nullptr || Character->IsElimed()) return;//要是角色不存在或者已经被淘汰了就不治疗了

	if(bSlowlyCharged)//是否开启瞬间恢复护盾
	{
		const float RechargeThisFrame = ChargingShiledRate * DeltaTime;//每一帧的回血量
		Character->SetShield(FMath::Clamp(Character->GetShield() + RechargeThisFrame, 0.f, Character->GetMaxShield()));//设置护盾的数值
		Character->UpdateHUDShield();//更新一次护盾HUD
		RechargedAmount+= RechargeThisFrame * AmountToRechargeShiled;//已经恢复的护盾的计数

		//FString Message1 = FString::SanitizeFloat(RechargedAmount);

		//if (GEngine)
		//{
		//	GEngine->AddOnScreenDebugMessage(
		//		-1,
		//		5,
		//		FColor::Yellow,
		//		Message1
		//	);
		//}
	
		if (Character->IsElimed() || Character->GetShield() == Character->GetMaxShield() || RechargedAmount == AmountToRechargeShiled) //如果治疗过程中玩家被淘汰了或者血量恢复到了最大值或者血量已经恢复到了最大的治疗效果但是还没回满都停止治疗
		{
			RechargedAmount = 0;//充能计数清零
			AmountToRechargeShiled = 0;
			bRecharge = false;//充能结束
			return;
		}
	}
	else//瞬间恢复护盾的话
	{
		Character->SetShield(FMath::Clamp(Character->GetShield() + AmountToRechargeShiled, 0.f, Character->GetMaxShield()));//设置护盾
		Character->UpdateHUDShield();
		AmountToRechargeShiled = 0;
		bRecharge = false;//充能结束
		return;
	}



}
