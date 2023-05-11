// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShells.generated.h"

UCLASS()
class BLASTERAGAIN_API ABulletShells : public AActor
{
	GENERATED_BODY()
	
public:	

	ABulletShells();

protected:

	virtual void BeginPlay() override;
	UFUNCTION()//受到打击的效果
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BulletShellMesh; 

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;//子弹壳弹出冲量

	UPROPERTY(EditAnywhere)
	class USoundCue* BulletSound;
public:	

	virtual void Tick(float DeltaTime) override;

};
