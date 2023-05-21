// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	USoundCue* RocketLoopInAir;//火箭在天上一直飞的呼呼声

	UPROPERTY()
	UAudioComponent* RocketLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;//火箭在天上飞的声音的衰减设置

	UPROPERTY(EditAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

private:


public:
	UPROPERTY(EditAnywhere)
	float InnerScope = 200.f;//武器的满伤害范围距离

	UPROPERTY(EditAnywhere)
	float OutScope = 500.f;//武器的减伤范围距离

	AProjectileRocket();

	virtual void Destroyed() override;


#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif

};
