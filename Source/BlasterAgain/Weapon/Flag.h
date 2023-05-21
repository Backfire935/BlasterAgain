// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	AFlag();

	virtual void Dropped() override;

	virtual void HandleWeaponEquiped() override;//处理装备过的武器
	virtual void HandleWeaponDropped() ;//处理丢弃的武器
	virtual void BeginPlay() override;
	void ResetFlag();
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;

	FTransform InitialTransform;

public:
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }
	
};
