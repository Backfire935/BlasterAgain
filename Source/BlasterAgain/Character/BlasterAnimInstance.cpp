// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include"GameFramework/CharacterMovementComponent.h"
#include "BlasterCharacter.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());

}

//帧运行
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if(BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	//设置速度
	Speed = Velocity.Size();
	//是否在空中
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	//是否在加速
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

}
