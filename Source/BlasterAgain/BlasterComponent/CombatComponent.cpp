
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

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bAiming);
}



void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}


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

