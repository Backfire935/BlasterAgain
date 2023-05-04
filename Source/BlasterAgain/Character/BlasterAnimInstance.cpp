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
	//�Ƿ�װ������
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	//װ��������
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	//�Ƿ����¶�
	bIsCrouched = BlasterCharacter->bIsCrouched;
	//�Ƿ�����׼
	bAiming = BlasterCharacter->IsAiming();

	//��ɫɨ��״̬�����Yawƫ��
	FRotator AimRotation =  BlasterCharacter->GetBaseAimRotation();//���������ת�Ƕ�
	FRotator MovementRotation =  UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());//��ɫ����ת�Ƕ�
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 5.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotatorLastFrame = CharacterRotator;
	CharacterRotator = BlasterCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotator, CharacterRotatorLastFrame);
	const float Target = Delta.Yaw / DeltaTime;

	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);//��ȡ������۵���ά��Ϣ
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));//����������const TQuat<T>& NewRotation ������Ҫ��using FQuat = UE::Math::TQuat<double>;
	}
}


