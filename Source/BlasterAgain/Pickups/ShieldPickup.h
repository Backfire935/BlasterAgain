// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API AShieldPickup : public APickup
{
	GENERATED_BODY()
public:
	AShieldPickup();
protected:

	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float ChargeAmount = 0.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float ChargingTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	bool bChargeSlowly = false;//是否缓慢恢复护盾
};
