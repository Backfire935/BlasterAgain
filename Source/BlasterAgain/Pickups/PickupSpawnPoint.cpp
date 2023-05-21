// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"

#include "Pickup.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

}


void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnPickupTimer((AActor*)nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();//得到所有可拾取buff类型的数量
	if(NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses-1);
		SpawnedPickup =  GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());//在指定位置随机生成一个可拾取类
		if(HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);//绑定一个多播委托，当此时的buff被拾取后，就调用绑定的StartSpawnPickupTimer函数重新计时生成。用的UE的委托，函数声明那需要添加UFUNCTION()反射
		}
	}

}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if(HasAuthority())
	{
		SpawnPickup();//生成buff函数,但是种类随机
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin,SpawnPickupTimeMax);//设置一个随机生成时间
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&APickupSpawnPoint::SpawnPickupTimerFinished,//Timer计时结束后调用生成buff函数
		SpawnTime
	);

}

