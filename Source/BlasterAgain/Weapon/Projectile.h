// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTERAGAIN_API AProjectile : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

	//用于服务端回溯
	bool bUseServerSideRewind = false;

	FVector_NetQuantize TraceStart;//只有整数没有小数
	FVector_NetQuantize100 InitialVelocity;//这种类型的参数对网络复制进行了优化,整体大小比FVector_NetQuantize小

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	//只在榴弹和火箭筒里设置这个
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	//对榴弹和火箭筒不起作用
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()//受到打击的效果
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void SpawnTrailSystem();//生成子弹的尾迹

	void StartDestroyTimer();//设置3s延迟销毁保证火箭弹的尾气不会在火箭弹发生碰撞的时候立即销毁
	
	void DestroyTimerFinished();

	void ExplodeDamage();


	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;	//子弹运动组件

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;//要选择的尾气粒子特效系统

	UPROPERTY()
	class  UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
private:

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;


	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;



};
