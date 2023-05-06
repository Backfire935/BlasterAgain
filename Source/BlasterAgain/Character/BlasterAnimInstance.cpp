// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include"GameFramework/CharacterMovementComponent.h"
#include "BlasterCharacter.h"
#include "BlasterAgain/Weapon/Weapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

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
	//是否装备武器
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	//装备的武器
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	//是否在下蹲
	bIsCrouched = BlasterCharacter->bIsCrouched;
	//是否在瞄准
	bAiming = BlasterCharacter->IsAiming();

	//角色扫射状态身体的Yaw偏移
	FRotator AimRotation =  BlasterCharacter->GetBaseAimRotation();//摄像机的旋转角度
	FRotator MovementRotation =  UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());//角色的旋转角度
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 4.f);//值越低动画切换越缓
	YawOffset = DeltaRotation.Yaw;

	CharacterRotatorLastFrame = CharacterRotator;
	CharacterRotator = BlasterCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotator, CharacterRotatorLastFrame);
	const float Target = Delta.Yaw / DeltaTime;

	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())//FABRIK IK
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);//获取武器插槽的三维信息
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));//参数类型是const TQuat<T>& NewRotation 所以需要用using FQuat = UE::Math::TQuat<double>;
	}
}


