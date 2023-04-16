// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include"GameFramework/CharacterMovementComponent.h"
#include "BlasterCharacter.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());

}

//֡����
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
	//�����ٶ�
	Speed = Velocity.Size();
	//�Ƿ��ڿ���
	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	//�Ƿ��ڼ���
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

}
