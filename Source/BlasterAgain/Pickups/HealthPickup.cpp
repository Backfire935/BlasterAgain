// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"

#include "BlasterAgain/BlasterComponent/BuffComponent.h"
#include "BlasterAgain/Character/BlasterCharacter.h"

AHealthPickup::AHealthPickup()
{
	bReplicates = true;
}



void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
									UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		if (BlasterCharacter->GetHealth() == BlasterCharacter->GetMaxHealth()) return;//满血状态下碰到无效
		UBuffComponent* Buff = BlasterCharacter->GetBuff();
		if(Buff)
		{
			Buff->Heal(HealAmount,HealingTime);
		}
	
	}
	Destroy();
}
