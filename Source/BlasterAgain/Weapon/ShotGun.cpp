#include "ShotGun.h"

#include "WeaponTypes.h"
#include "BlasterAgain/BlasterComponent/LagCompensationComponent.h"
#include "BlasterAgain/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterAgain/PlayerController/BlasterPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
void AShotGun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)//一个直线武器攻击检测
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();//开火检测的起始点

		//对应命中的角色和被命中的次数
		TMap<ABlasterCharacter*, uint32> HitMap;//将被命中的对象都加进去
		TMap<ABlasterCharacter*, uint32> HeadShotHitMap;//将被爆头的对象都加进去


		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());//获取击中的目标
			if (BlasterCharacter )//在本地执行，不检查权威性和造成伤害的玩家控制器
			{
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

				if(bHeadShot)
				{
					if (HeadShotHitMap.Contains(BlasterCharacter)) HeadShotHitMap[BlasterCharacter]++;//已经击中过这个人Hits就+1
					else HeadShotHitMap.Emplace(BlasterCharacter, 1);//以前没击中过这个人就添加进去并且置Hits为1
				}
				//没被爆头的情况下
				else
				{
					if (HitMap.Contains(BlasterCharacter)) HitMap[BlasterCharacter]++;//已经击中过这个人Hits就+1
					else HitMap.Emplace(BlasterCharacter, 1);//以前没击中过这个人就添加进去并且置Hits为1
				}
				

				if (ImpactParticles)//生成打击的贴花效果
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}

				if (HitSound)//生成命中音效
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint,
						.5f,
						FMath::FRandRange(-0.5F, 0.5F)
					);
				}

			}

		}

		TArray<ABlasterCharacter*> HitCharacters;//角色表
		TMap<ABlasterCharacter*, float> DamageMap;//角色伤害表

		//开始计算每个角色被命中的总伤害
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);//命中的弹药*每一发弹药的伤害
				HitCharacters.AddUnique(HitPair.Key);//只会添加不在里面的玩家
			}
		}

		//开始计算每个角色被爆头的总伤害
		for (auto HeadShotHitPair : HeadShotHitMap)
		{
			if (HeadShotHitPair.Key)
			{
				//伤害表已经被记录过爆头了，就把这次新的爆头伤害加上
				if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				//伤害表头一次遇到这个玩家就加进去，顺便记录第一次受到的伤害
				else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);
				//如果是头一次遇到这个玩家，玩家表里记录一下
				HitCharacters.AddUnique(HeadShotHitPair.Key);//只会添加不在里面的玩家
			}
		}

		//计算这一喷子对每个玩家的总伤害
		for(auto DamagePair : DamageMap)
		{
			if(InstigatorController && DamagePair.Key)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				//不使用服务器回溯就直接算伤害,目前服务端开了这个功能就无法对客户端造成伤害
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key,//受到伤害的玩家
						DamagePair.Value,//受到伤害的值
						InstigatorController,//伤害来源的控制器
						this,//伤害来源
						UDamageType::StaticClass()
					);
				}
			}
		}

	

		//是否使用了服务器回溯,客户端才有这个选项
		if (!HasAuthority() && bUseServerSideRewind)
		{
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
			BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
			if (BlasterOwnerCharacter && BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensationComponent() && BlasterOwnerCharacter->IsLocallyControlled())
			{
				BlasterOwnerCharacter->GetLagCompensationComponent()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime
				);
			}
		}

	}
}

void AShotGun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)//获取喷子子弹群的落点
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) return;//一个直线武器攻击检测

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();//开火检测的起始点

	const FVector  ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//一个从射线起始点到被击中目标的向量
	const FVector  SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;//到喷子射程终点的中点向量


	for(uint32 i =0; i < NumberOfPellets; i++)//设置循环大小为一次发射子弹的数量
	{
		const FVector  RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);//随机方向单位向量*随机长度 
		const FVector EndLoc = SphereCenter + RandVec;//中心到四周的随机扩散向量
		 FVector ToEndLoc = EndLoc - TraceStart;//两点间的线段
		 FVector EndEnd = (TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());//单个射线向量

		HitTargets.Add(EndEnd);
	}
}
