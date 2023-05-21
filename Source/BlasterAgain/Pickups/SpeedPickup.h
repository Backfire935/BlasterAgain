// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API ASpeedPickup : public APickup
{
	GENERATED_BODY()
public:
	ASpeedPickup();

	UPROPERTY(EditAnywhere)
	float  SpeedUpTime = 10.f;//速度buff 十秒
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

	float CurrentWalkSpeed = 0.f;
	float CurrentCrouchSpeed = 0.f;

	UPROPERTY(EditAnywhere)
	float AcclerateToWalk = 0.f;

	UPROPERTY(EditAnywhere)
	float AcclerateToCrouch= 0.f;
};
