// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"

#include "BlasterAgain/GameMode/CaptureTheFlagGameMode.h"
#include "BlasterAgain/Weapon/Flag.h"
#include "Components/SphereComponent.h"

AFlagZone::AFlagZone()
{

	PrimaryActorTick.bCanEverTick = false;

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);

}


void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	//当组件开始覆盖的时候，绑定事件
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	//放置旗子的点
	if(OverlappingFlag && OverlappingFlag->GetTeam() == Team)
	{
		ACaptureTheFlagGameMode* GameMode =  GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if(GameMode)
		{
			//进了放旗子的点就加分
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		//重置旗子
		OverlappingFlag->ResetFlag();
	}
}