

#include "LagCompensationComponent.h"

#include "BlasterAgain/BlasterAgain.h"
#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}



void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	
}


void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFramePackage();
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderPackage,
	const FFramePackage& YoungerPackage, float HitTime)
{
	const float Distance =  (YoungerPackage.time - OlderPackage.time);//两个包之间的时间
	const float InterpFraction = FMath::Clamp(HitTime - OlderPackage.time / Distance, 0 ,1);// 命中到上一个包之间的时间/两个包之间的时间

	FFramePackage InterpFramePackage;
	InterpFramePackage.time = HitTime;

	for(auto& YoungerPair : YoungerPackage.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderPackage.HitBoxInfo[BoxInfoName];//获取value
		const FBoxInformation& YoungerBox = YoungerPackage.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;//模拟计算出来的客户端命中时的信息位置
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location,YoungerBox.Location, 1.f, InterpFraction);//插值算出来的location信息
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);//插值算出来的rotation信息
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;//盒体大小

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);//封包
	}

	return InterpFramePackage;

}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter ==nullptr ) return FServerSideRewindResult();
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);//缓存角色当前的位置信息到本地包
	MoveBoxes(HitCharacter, Package);//移动hit盒子到客户端发来的包的位置信息
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);//关掉角色模型的碰撞,避免下面和hitbox碰撞的逻辑出问题

	//判断射线与hitbox的碰撞检测逻辑
	//先打开与头部hitbox的碰撞
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//碰撞开关
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);//碰撞种类

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25;
	UWorld *World = GetWorld();
	if(World)
	{//射线碰撞检测，因为只开了头部的碰撞，所以只要碰到了那就是碰到头了
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if(ConfirmHitResult.bBlockingHit)//命中头部，提前结束
		{
			if(ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if(Box)
				{
					DrawDebugBox(GetWorld(),Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false,
						8.f);
				}
			}
			ResetHitBoxes(HitCharacter, CurrentFrame);//将角色的位置移回去
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
			return FServerSideRewindResult{ true , true };
		}
		//没有命中头部的话，就去检测剩下的身体部位
		if (!ConfirmHitResult.bBlockingHit)
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					//已经检测过头部了，如果能命中函数已经return，能运行到这就肯定不能命中头部，所以直接开启所有的碰撞即可，而且返回的结果已经被确定了
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
				World->LineTraceSingleByChannel(
					ConfirmHitResult,
					TraceStart,
					TraceEnd,
					ECC_HitBox
				);
				if(ConfirmHitResult.bBlockingHit)
				{
					if (ConfirmHitResult.Component.IsValid())
					{
						UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
						if (Box)
						{
							DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false,
								8.f);
						}
					}
					ResetHitBoxes(HitCharacter, CurrentFrame);//将角色的位置移回去
					EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
					return FServerSideRewindResult{ true , false };
				}
			
		}
	}
	//如果检测后确定没有命中目标,关掉所有属性，返回两个false
	ResetHitBoxes(HitCharacter, CurrentFrame);//将角色的位置移回去
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
	return FServerSideRewindResult{ false , false };
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);//缓存角色当前的位置信息到本地包
	MoveBoxes(HitCharacter, Package);//移动hit盒子到客户端发来的包的位置信息
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);//关掉角色模型的碰撞,避免下面和hitbox碰撞的逻辑出问题

	//判断射线与hitbox的碰撞检测逻辑
	//先打开与头部hitbox的碰撞
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//碰撞开关
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);//碰撞种类

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = 15.f;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	//命中头部就直接返回
	if(PathResult.HitResult.bBlockingHit)
	{
		if (PathResult.HitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if (Box)
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false,
					8.f);
			}
		}
		ResetHitBoxes(HitCharacter, CurrentFrame);//将角色的位置移回去
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
		return FServerSideRewindResult{ true , true };
	}
	//未命中头部，检查身体剩余部位
	if(!PathResult.HitResult.bBlockingHit)
	{
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				//已经检测过头部了，如果能命中函数已经return，能运行到这就肯定不能命中头部，所以直接开启所有的碰撞即可，而且返回的结果已经被确定了
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if(PathResult.HitResult.bBlockingHit)
		{
			if (PathResult.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false,
						8.f);
				}
			}
			ResetHitBoxes(HitCharacter, CurrentFrame);//将角色的位置移回去
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
			return FServerSideRewindResult{ true , false };
		}
	}

	//确定啥也没打到
	ResetHitBoxes(HitCharacter, CurrentFrame);//将角色的位置移回去
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
	return FServerSideRewindResult{ false , false };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages,
                                                                            const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	//喷子一枪下来好多发子弹，所以这里的操作都是for遍历

	for (auto& Frame : FramePackages)//先检查包，要是包是空的说明传输过程出了问题就不执行了。
	{
		if (Frame.Character == nullptr) return FShotgunServerSideRewindResult();
	}

	FShotgunServerSideRewindResult ShotgunResult;
	TArray<FFramePackage> CurrentFrames;

	//缓存所有打出来的子弹的包位置
	for(auto & Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		CacheBoxPositions(Frame.Character, CurrentFrame);//缓存角色当前的位置信息到本地包
		MoveBoxes(Frame.Character, Frame);//移动hit盒子到客户端发来的包的位置信息
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);//关掉角色模型的碰撞,避免下面和hitbox碰撞的逻辑出问题
		CurrentFrames.Add(CurrentFrame);
	}

	//开启每个包的头部盒子碰撞 
	for(auto& Frame : FramePackages)
	{
		//判断射线与hitbox的碰撞检测逻辑
//先打开与头部hitbox的碰撞
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);//碰撞开关
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);//碰撞种类
	}

	UWorld* World = GetWorld();
	//爆头检测
	for(auto & HitLocation : HitLocations)//每个包进行碰撞检测
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25;
		if(World)
		{
			//射线碰撞检测，因为只开了头部的碰撞，所以只要碰到了那就是碰到头了
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());//获取击中的目标
			if(BlasterCharacter)
			{
				if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false,
							8.f);
					}
				}

				if (ShotgunResult.HeadShots.Contains(BlasterCharacter))//已经击中过这个人Hits就+1
				{
					ShotgunResult.HeadShots[BlasterCharacter]++;
				}
				else//以前没击中过这个人就添加进去并且置Hits为1
				{
					ShotgunResult.HeadShots.Emplace(BlasterCharacter, 1);
				}
			}

		}

	}

	//开启全身的碰撞盒子后关闭头部的碰撞盒子
	for(auto & Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		//关闭每个包的头部盒子碰撞
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);//碰撞开关

	}

	//命中身体检测
	for (auto& HitLocation : HitLocations)//每个包进行碰撞检测
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25;
		if (World)
		{
			//射线碰撞检测，因为只开了头部的碰撞，所以只要碰到了那就是碰到头了
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());//获取击中的目标
			if (BlasterCharacter)
			{
				if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false,
							8.f);
					}
				}
				if (ShotgunResult.BodyShots.Contains(BlasterCharacter))//已经击中过这个人Hits就+1
				{
					ShotgunResult.BodyShots[BlasterCharacter]++;
				}
				else//以前没击中过这个人就添加进去并且置Hits为1
				{
					ShotgunResult.BodyShots.Emplace(BlasterCharacter, 1);
				}
				
			}

		}
		
	}
	

	//如果检测后确定没有命中目标,关掉所有属性，返回两个false
	for(auto & Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);//将角色的位置移回去
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);//检测完了就打开模型碰撞，不然游戏要出事
	}
	

	return ShotgunResult;
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr) return;
	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxes)//遍历角色全身的碰撞盒子
	{
		if(HitBoxPair.Value != nullptr )
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key,BoxInfo);//封包,每一个盒子的名字和盒子的信息
		}
	}
}
 
void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr)return;
	for(auto &HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if(HitBoxPair.Value != nullptr)//value是具体的碰撞盒子
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);//设置组件的位置信息到
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);//设置组件的旋转信息到
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);//设置组件的大小
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)//将角色的位置移回去
{
	if (HitCharacter == nullptr)return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)//value是具体的碰撞盒子
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);//设置组件的位置信息到
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);//设置组件的旋转信息到
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);//设置组件的大小
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);//直接关掉head hit box碰撞
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if(HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}


void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)//显示包内容
{
	for(auto & BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			FramePackageShowTime
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)//发送信息给服务端回溯的函数
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
	
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return  ProjectileConfirmHit(FrameToCheck ,HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck;
	for(ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter,HitTime));
	}

	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character && HitCharacter && Character->GetEquippedWeapon() && Confirm.bHitComfirmed)
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}


FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FFramePackage();

	bool bShouldInterpolate = true;//是否使用插帧计算

	//我们用于检查命中的帧包
	FFramePackage FrameToCheck;

	//被命中的玩家的帧历史
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	//帧历史中最后一帧发生的时间
	const float OldestHistoryTime = HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail()->GetValue().time;
	//帧历史中最新一帧发生的时间
	const float NewestHistoryTime = HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead()->GetValue().time;

	if (OldestHistoryTime > HitTime)//如果命中时间发生比最早的纪录还早，本次命中无效，因为已经超出回溯的范围了
	{
		return FFramePackage();
	}

	if (OldestHistoryTime == HitTime)//如果命中的时间恰好等于最后一帧包的时间
	{
		FrameToCheck = History.GetTail()->GetValue();//要检查的帧包为最后一个帧包
		bShouldInterpolate = false;
	}

	if (NewestHistoryTime <= HitTime)//如果最新存储的时间比命中的时间还小,听起来不可能发生但是由于延迟，说不准，反正用第一个包去判断就是了
	{
		FrameToCheck = History.GetHead()->GetValue();//直接检查第一个帧包
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().time > HitTime)//如果当前older节点的时间比命中时间还晚
	{
		if (Older->GetNextNode() == nullptr) break;//如果下个节点是空节点的话，就结束循环回到函数
		//Younger = Older;//让young节点移到此处
		//Older = Older->GetNextNode();//让older节点往前移
		Older = Older->GetNextNode();
		if (Older->GetValue().time > HitTime)
		{
			Younger = Older;
		}
	}//最后的结果是要older节点时间比hittime早，young节点时间比hittime晚

	//这种情况几乎不可能但还是加上
	if (Older->GetValue().time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if (bShouldInterpolate)
	{
		//这里调用插帧函数
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}



void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,const FVector_NetQuantize& HitLocation,float HitTime)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if(Character && HitCharacter && Character->GetEquippedWeapon() && Confirm.bHitComfirmed)
	{
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();

		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters,const FVector_NetQuantize& TraceStart,const TArray<FVector_NetQuantize>& HitLocations,float HitTime
)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters , TraceStart , HitLocations, HitTime);

	for(auto & HitCharacter : HitCharacters)
	{
		if(HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || Character == nullptr) continue;
		float TotalDamage=0.f;
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
		  	float HeadShotDamage = Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;

		}
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
		     float BodyShotDamage = Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;

		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			HitCharacter->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	//	FString RemoteRoleString = FString::Printf(TEXT("hit :%f"), TotalDamage);
		
		UE_LOG(LogTemp, Warning, TEXT("%f"), TotalDamage);

	}
}



void ULagCompensationComponent::SaveFramePackage()
{
	//只有服务端才能执行
	if (Character == nullptr || !Character->HasAuthority())return;

	if (FrameHistory.Num() <= 1)//第一帧的情况下
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);//存下第一帧
		FrameHistory.AddHead(ThisFrame);//添加到链表中
		
	}
	else//添加了一帧以上的情况
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().time - FrameHistory.GetTail()->GetValue().time;//存储当前的帧长度为最新的时间-最旧的时间，后面和总时长比较
		while (HistoryLength > MaxRecordTime)//当前的帧长度>最大纪录时间的话，丢掉超出最大纪录时间的部分
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().time - FrameHistory.GetTail()->GetValue().time;
		}
		//丢掉不需要的后，就可以继续更新新的内容
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);//获取当前帧
		FrameHistory.AddHead(ThisFrame);//当前帧添加到链表中

		ShowFramePackage(ThisFrame, FColor::Blue);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)//帧打包
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character && Character->HitCollisionBoxes.Num() > 0)
	{
		Package.time = GetWorld()->GetTimeSeconds();//帧包中添加时间
		Package.Character = Character;
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			if (BoxPair.Value != nullptr)//反正因为此时获得的BoxPair.Value为空而导致编辑器奔溃，修了一天
			{
				FBoxInformation  BoxInformation;
				BoxInformation.Location = BoxPair.Value->GetComponentLocation();//Value就是组件
				BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
				BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();//获取组件盒体大小
				//打包信息
				Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);// HitBoxInfo的结构是TMap<FName, FBoxInformation>，所以使用BoxPair.Key
			}
			
		}
	}
}
