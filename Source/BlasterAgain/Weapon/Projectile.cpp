// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "NiagaraFunctionLibrary.h"
#include "BlasterAgain/BlasterAgain.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);//设置碰撞通道
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//设置仅物理碰撞
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);



}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);//生成开火轨迹
	}
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this , &AProjectile::OnHit);//添加子弹撞击时的打击效果
	}

//	CollisionBox->IgnoreActorWhenMoving();//当角色在移动的时候忽略碰撞，避免自己向前走的时候发射导弹把自己炸死,但是这个办法不好用，因为你会更早的把自己炸死
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	Destroy();
}

void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,//要生成的粒子特效
			GetRootComponent(),//要附加在哪个组件上
			FName(),//要附加在组件的哪个插槽上,这里不设置传空
			GetActorLocation(),//要生成的位置信息
			GetActorRotation(),//要生成的旋转信息
			EAttachLocation::KeepWorldPosition,//要附加的位置类型
			false//是否要自动销毁，这里希望手动设置
		);//得到的是一个粒子特效组件
	}
}


// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());//在被击中的位置放置一个枪痕的贴花
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());//在被击中的位置播放一个被命中的音效
	}

}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);//设置3s延迟销毁保证火箭弹的尾气不会在火箭弹发生碰撞的时候立即销毁

}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::ExplodeDamage()
{

	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)//应用一个球形范围伤害
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,//世界上下文对象 
				Damage,//基本伤害
				0.3 * Damage,//最低伤害
				GetActorLocation(),//在击中的位置设为伤害的中心
				DamageInnerRadius,//满伤害距离
				DamageOuterRadius,//减伤距离
				1.f,//伤害衰减计算是指数计算，设置为1后可以得到一个按距离的线性伤害衰减
				UDamageType::StaticClass(),//伤害类型
				TArray<AActor*>(),//忽略对指定类型的物体的伤害
				this,//产生伤害的对象
				FiringController//谁开火的
			);//带距离衰减的球形范围伤害
		}
	}
}
