
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
	if(Character && bAiming)//是否在瞄准
	{
		if(Character->GetIsCrouch())//是否在下蹲
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchAimSpeed();//设置成下蹲瞄准的速度
		}
		else//站着
		{
				Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunAimSpeed() : Character->GetWalkAimSpeed();//是不是在奔跑 是设置成奔跑瞄准的速度  不是设置成行走瞄准的速度
		}
	}
	else if(Character && !bAiming)//不再瞄准了
	{
		if (Character->GetIsCrouch())//在下蹲
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchSpeed();//恢复成下蹲的行走速度
		}
		else//站着
		{
				Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunSpeed() : Character->GetWalkSpeed();//是不是在奔跑，是设置成奔跑的速度 不是设置成行走的速度
		}
	}
}



void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character && bAiming)//是否在瞄准
	{
		if (Character->GetIsCrouch())//是否在下蹲
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchAimSpeed();//设置成下蹲瞄准的速度
		}
		else//站着
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunAimSpeed() : Character->GetWalkAimSpeed();//是不是在奔跑 是设置成奔跑瞄准的速度  不是设置成行走瞄准的速度
		}
	}
	else if (Character && !bAiming)//不再瞄准了
	{
		if (Character->GetIsCrouch())//在下蹲
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchSpeed();//恢复成下蹲的行走速度
		}
		else//站着
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunSpeed() : Character->GetWalkSpeed();//是不是在奔跑，是设置成奔跑的速度 不是设置成行走的速度
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
	//关闭模拟物理
	EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(false);
	//关闭重力
	EquippedWeapon->GetWeaponMesh()->SetEnableGravity(false);
	EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞检测

	const USkeletalMeshSocket * HandSocket =  Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh()); //在插槽上将武器附加到身体上
	}
	EquippedWeapon->SetOwner(Character);
	//Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	//Character->bUseControllerRotationYaw = true;
}



void UCombatComponent::DropWeapon()
{
	if (EquippedWeapon)//如果武器存在
	{
		EquippedWeapon->Dropped();//如果已经装备了一件武器，就丢掉手上的
		EquippedWeapon->SetOwner(nullptr);//将拥有者设置为空
		EquippedWeapon = nullptr;
	}
}

