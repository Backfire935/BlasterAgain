// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTERAGAIN_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealingTime);

	void RechargeShiled(float RechargeAmount, float RechargingTime , bool bSlowly);

	void SpeedUp (float DefaultWalkSpeed,float DefaultCrouchSpeed,float AcclerateSpeed, float AcclerateCrouch,float SpeedTime) ;

	void SpeedDown();

	void JumpUp(float LastJumpVelocity,float JumpVelocity, float JumpupTime);
	
protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);

	void ChargeRampUp(float DeltaTime);


public:	
	UPROPERTY()
	class ABlasterCharacter* Character;

	//用于治疗buff
	bool bHealing = false;//是否开启回血状态

	float HealingRate = 0;//回血速率

	float AmountToHeal = 0;//回血总量

	float HealedAmount = 0;//用于存储已经恢复的血量

	//用于护盾buff
	bool bRecharge = false;//是否开启恢复护盾状态

	float ChargingShiledRate = 0;//恢复护盾速率

	float AmountToRechargeShiled = 0;//恢复护盾总量

	float RechargedAmount = 0;//用于存储已经恢复的护盾

	bool bSlowlyCharged = false;//是缓慢恢复护盾还是一次充满

	//用于加速buff
	float LastWalkSpeed = 0.f;
	float LastCrouchSpeed = 0.f;

	FTimerHandle SpeedUpTimer;//加速的计时器

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	//用于跳跃buff
	FTimerHandle JumpBuff;
	
	void ResetJump();

	float InitialJumpZVelocity;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastJumpBuff(float JumpVelocity);
};
