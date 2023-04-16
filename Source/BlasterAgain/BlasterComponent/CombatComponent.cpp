
#include "CombatComponent.h"

#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/Weapon/Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	//关闭重力
	EquippedWeapon->GetWeaponMesh()->SetEnableGravity(false);
	EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞检测
	//关闭模拟物理
	EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(false);

	const USkeletalMeshSocket * HandSocket =  Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh()); //在插槽上将武器附加到身体上
	}
	EquippedWeapon->SetOwner(Character);

}



void UCombatComponent::DropWeapon()
{
	if (EquippedWeapon)//如果武器存在
	{
		EquippedWeapon->Dropped();//如果已经装备了一件武器，就丢掉手上的
		EquippedWeapon = nullptr;
	}
}

