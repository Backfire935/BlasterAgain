// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "BlasterAgain/BlasterComponent/LagCompensationComponent.h"
#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/PlayerController/BlasterPlayerController.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);//将组件设置为可网络复制
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}

}
#endif


void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ABlasterCharacter * OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		ABlasterPlayerController* OwnerController =Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			if(OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				const 	float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());//对目标actor传递伤害
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if(bUseServerSideRewind && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(
					HitCharacter,
					TraceStart,
					InitialVelocity,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime
				);
			}
		} 
		
	}

	Super::OnHit(  HitComp,  OtherActor, OtherComp,  NormalImpulse,  Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	/*
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;//使用特定碰撞通道来跟踪
	PathParams.bTraceWithCollision = true;//允许预测投射路径，在世界中追踪碰撞物体,可以生成命中事件
	PathParams.DrawDebugTime = 5.f;//持续5s的时间
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PathParams.MaxSimTime = 4.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 30.f;
	PathParams.StartLocation = GetActorLocation();
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	PathParams.ActorsToIgnore.Add(this);


	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	*/
}
