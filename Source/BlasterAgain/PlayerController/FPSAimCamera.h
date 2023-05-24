// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSAimCamera.generated.h"

UCLASS()
class BLASTERAGAIN_API AFPSAimCamera : public AActor
{
	GENERATED_BODY()
	
public:	

	AFPSAimCamera();

protected:

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* AimCamera;
	
	virtual void BeginPlay() override;

public:	

	virtual void Tick(float DeltaTime) override;

	FORCEINLINE UCameraComponent* GetAimCamera() { return AimCamera;}
	
};
