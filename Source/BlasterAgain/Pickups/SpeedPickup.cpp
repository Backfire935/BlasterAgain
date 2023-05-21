// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"

#include "BlasterAgain/BlasterComponent/BuffComponent.h"
#include "BlasterAgain/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

//使用前先在蓝图里检查下默认值是否设置，C++中的速度默认值是0，小心一会碰了动不了
ASpeedPickup::ASpeedPickup()
{
	bReplicates = true;
}

void ASpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		//if (BlasterCharacter->GetHealth() == BlasterCharacter->GetMaxHealth()) return;//满血状态下碰到无效
		UBuffComponent* Buff = BlasterCharacter->GetBuff();
		CurrentWalkSpeed = BlasterCharacter->GetCharacterMovement()->GetMaxSpeed();//获取当前速度
		CurrentCrouchSpeed = BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched;
		if (Buff)
		{
			Buff->SpeedUp(CurrentWalkSpeed,CurrentCrouchSpeed, AcclerateToWalk, AcclerateToCrouch,SpeedUpTime);//这个accleratespeed必须先在蓝图设置好，不然触发后默认速度是0
		}
	}
	Destroy();
}
