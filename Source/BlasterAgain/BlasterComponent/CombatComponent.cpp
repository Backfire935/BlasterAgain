
#include "CombatComponent.h"

#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

#pragma region Aiming
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bAiming);
	if(Character && bAiming)//�Ƿ�����׼
	{
		if(Character->GetIsCrouch())//�Ƿ����¶�
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchAimSpeed();//���ó��¶���׼���ٶ�
		}
		else//վ��
		{
				Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunAimSpeed() : Character->GetWalkAimSpeed();//�ǲ����ڱ��� �����óɱ�����׼���ٶ�  �������ó�������׼���ٶ�
		}
	}
	else if(Character && !bAiming)//������׼��
	{
		if (Character->GetIsCrouch())//���¶�
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchSpeed();//�ָ����¶׵������ٶ�
		}
		else//վ��
		{
				Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunSpeed() : Character->GetWalkSpeed();//�ǲ����ڱ��ܣ������óɱ��ܵ��ٶ� �������ó����ߵ��ٶ�
		}
	}
}



void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character && bAiming)//�Ƿ�����׼
	{
		if (Character->GetIsCrouch())//�Ƿ����¶�
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchAimSpeed();//���ó��¶���׼���ٶ�
		}
		else//վ��
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunAimSpeed() : Character->GetWalkAimSpeed();//�ǲ����ڱ��� �����óɱ�����׼���ٶ�  �������ó�������׼���ٶ�
		}
	}
	else if (Character && !bAiming)//������׼��
	{
		if (Character->GetIsCrouch())//���¶�
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchSpeed();//�ָ����¶׵������ٶ�
		}
		else//վ��
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunSpeed() : Character->GetWalkSpeed();//�ǲ����ڱ��ܣ������óɱ��ܵ��ٶ� �������ó����ߵ��ٶ�
		}
	}
}
#pragma endregion Aiming

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	//�ر�ģ������
	EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(false);
	//�ر�����
	EquippedWeapon->GetWeaponMesh()->SetEnableGravity(false);
	EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // �ر���ײ���

	const USkeletalMeshSocket * HandSocket =  Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh()); //�ڲ���Ͻ��������ӵ�������
	}
	EquippedWeapon->SetOwner(Character);
	//Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	//Character->bUseControllerRotationYaw = true;
}



void UCombatComponent::DropWeapon()
{
	if (EquippedWeapon)//�����������
	{
		EquippedWeapon->Dropped();//����Ѿ�װ����һ���������Ͷ������ϵ�
		EquippedWeapon->SetOwner(nullptr);//��ӵ��������Ϊ��
		EquippedWeapon = nullptr;
	}
}

