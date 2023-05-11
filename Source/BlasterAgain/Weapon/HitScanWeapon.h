// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

protected:
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	void WeaponTraceHit(const FVector & TraceStart, const FVector& HitTarget, FHitResult& OutHit);//武器击中跟踪
};
