// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickup.h"

#include "BlasterAgain/BlasterComponent/BuffComponent.h"
#include "BlasterAgain/Character/BlasterCharacter.h"

AShieldPickup::AShieldPickup()
{
	bReplicates = true;
}

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		if (BlasterCharacter->GetShield() == BlasterCharacter->GetMaxShield()) return;//满盾状态下碰到无效
		UBuffComponent* Buff = BlasterCharacter->GetBuff();
		if (Buff)
		{
			Buff->RechargeShiled(ChargeAmount, ChargingTime , bChargeSlowly);
		}

	}
	Destroy();
}