// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "BlasterAgain/BlasterComponent/LagCompensationComponent.h"
#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)//一个直线武器攻击检测
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();//开火检测的起始点

		FHitResult FireHit;

		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());//获取击中的目标
		if (BlasterCharacter && InstigatorController)
		{
			//不使用服务器回溯就直接算伤害,目前服务端开了这个功能就无法对服务端造成伤害
			//if(HasAuthority() && !bUseServerSideRewind)
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				//判断是普通伤害还是爆头伤害
			 const 	float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				UGameplayStatics::ApplyDamage(
					BlasterCharacter,
					DamageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			 if(!HasAuthority() && bUseServerSideRewind)//是否使用了服务器回溯,客户端才有这个选项
			{
				BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
				BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
				if(BlasterOwnerCharacter && BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensationComponent() && BlasterOwnerCharacter->IsLocallyControlled())
				{
					BlasterOwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
						BlasterCharacter,
						Start,
						HitTarget,
						BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime
					);
				}
			}

		}
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}


void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart,const FVector & HitTarget, FHitResult& OutHit)
{

	UWorld* World = GetWorld();
	if(World)
	{
		FVector End =  TraceStart + (HitTarget - TraceStart) * 1.25;//1.25倍目标点到此的攻击距离

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		FVector BeamEnd = End;
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}

	}
}
