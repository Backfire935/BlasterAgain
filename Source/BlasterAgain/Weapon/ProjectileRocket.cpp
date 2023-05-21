// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "NiagaraComponent.h"
#include "RocketMovementComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);//将组件设置为可网络复制
	RocketMovementComponent->InitialSpeed = InitialSpeed;
	RocketMovementComponent->MaxSpeed = InitialSpeed;
}

void AProjectileRocket::Destroyed()
{

	Super::Destroyed();
}

#if WITH_EDITOR
void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);
	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(URocketMovementComponent, InitialSpeed))
	{
		if (RocketMovementComponent)
		{
			RocketMovementComponent->InitialSpeed = InitialSpeed;
			RocketMovementComponent->MaxSpeed = InitialSpeed;
		}
	}

}
#endif

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);//添加子弹撞击时的打击效果
	}

	SpawnTrailSystem();//生成尾迹

	if(RocketLoopInAir && LoopingSoundAttenuation)//生成导弹的呼呼声
	{
		RocketLoopComponent = UGameplayStatics::SpawnSoundAttached(
				RocketLoopInAir,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}


void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor == GetOwner())//如果判断出要炸的目标是自己的话
	{
		return;
	}
	ExplodeDamage();//应用伤害

	StartDestroyTimer();//设置3s延迟销毁保证火箭弹的尾气不会在火箭弹发生碰撞的时候立即销毁

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());//在被击中的位置放置一个枪痕的贴花
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());//在被击中的位置播放一个被命中的音效
	}

	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);//先设置模型不可见，过3s再被函数销毁模型实体
	}

	if(CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);//同时设置火箭弹的碰撞盒子为不可碰撞
	}

	if(TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController()) 
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();//不再激活粒子系统，就不会继续突突突冒尾气
	}

	if(RocketLoopComponent && RocketLoopComponent->IsPlaying())//如果撞到目标后还在播放声音
	{
		RocketLoopComponent->Stop();//停止播放呼呼声
	}
//	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
