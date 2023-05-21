// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);//调用父类的开火动画函数

	//if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket * MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));//获取武器枪口的插槽
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)//枪口插槽的位置
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());//获取当前手上的武器的枪口插槽
		
		//从命中目标-枪口插槽的向量
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;	
		SpawnParams.Owner = GetOwner();//设置发射物的拥有者
		SpawnParams.Instigator = InstigatorPawn;//发起对象

		AProjectile* SpawnedProjectile = nullptr;
		
		if(bUseServerSideRewind)
		{
			if(InstigatorPawn->HasAuthority())//Server使用ssr的情况下
			{
				if(InstigatorPawn->IsLocallyControlled())//服务端主机使用可复制的抛射物的情况
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass,SocketTransform.GetLocation(),TargetRotation,SpawnParams);//发射的弹药类，发射的插槽的位置，目标的旋转向量，发射物的一系列信息
					SpawnedProjectile->bUseServerSideRewind = false;//服务端没必要用SSR
					SpawnedProjectile->Damage = Damage;//弹药的伤害 = 武器的伤害
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				}
				else//服务端上的非本机控制的模拟控制器,生成不进行网络复制的抛射物(客户端RPC之后紧跟的服务端rep_notify已经复制了一遍了)，使用SSR
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);//发射的弹药类，发射的插槽的位置，目标的旋转向量，发射物的一系列信息
					SpawnedProjectile->bUseServerSideRewind = true;//服务端使用SSR避免造成双倍伤害 ProjectileBullet_if(OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
					SpawnedProjectile->Damage = Damage;//弹药的伤害 = 武器的伤害
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				}
			}
			else//客户端使用ssr的情况下
			{
				if (InstigatorPawn->IsLocallyControlled())//客户端本地控制的情况下，生成非复制的抛射物，使用ssr
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);//发射的弹药类，发射的插槽的位置，目标的旋转向量，发射物的一系列信息
					SpawnedProjectile->bUseServerSideRewind = true;//客户端的子弹这个时候可以用ssr了
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
					SpawnedProjectile->Damage = Damage;//弹药的伤害 = 武器的伤害
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;

				}
				else//客户端非本地控制，生成不进行网络复制的抛射物，不使用ssr
				{
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);//发射的弹药类，发射的插槽的位置，目标的旋转向量，发射物的一系列信息
					SpawnedProjectile->bUseServerSideRewind = false;//服务端没必要用SSR
					SpawnedProjectile->Damage = Damage;//弹药的伤害 = 武器的伤害
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				}
			}
		}
		else//武器不使用ssr的情况
		{
			if(InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);//发射的弹药类，发射的插槽的位置，目标的旋转向量，发射物的一系列信息
				
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;//弹药的伤害 = 武器的伤害
				SpawnedProjectile->HeadShotDamage = HeadShotDamage;
			}
		}
	}

}
