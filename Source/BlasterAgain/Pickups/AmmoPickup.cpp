// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"

#include "BlasterAgain/BlasterComponent/CombatComponent.h"
#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/Weapon/Weapon.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		if(!BlasterCharacter->IsWeaponEquipped())//如果没装备武器，就无事发生
			{
				return;
			}
		if(BlasterCharacter->GetEquippedWeapon()->GetWeaponType() != WeaponType )//如果拾取的弹药和手上的武器种类不同，就无事发生
			{
				return;
			} 
		UCombatComponent* Combat = BlasterCharacter->GetCombat();
		if(Combat)
		{
			Combat->PickupAmmo(WeaponType,AmmoAmount);//传入需要添加的武器弹药类型和弹药数目
		}
		
	}
	Destroy();
}
