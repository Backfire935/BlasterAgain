#pragma once

#include "BlasterCharacter.h"
#include "EnhancedInputSubsystems.h"
#include"EnhancedInputComponent.h"
#include "BlasterAgain/BlasterComponent/BuffComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"
#include"Components/WidgetComponent.h"
#include "BlasterAgain/Weapon/Weapon.h"
#include "BlasterAgain/BlasterComponent/CombatComponent.h"
#include "BlasterAgain/BlasterComponent/LagCompensationComponent.h"
#include "BlasterAgain/BlasterTypes/CombatState.h"
#include "BlasterAgain/BlasterTypes/Team.h"
#include "BlasterAgain/BlasterTypes/TurningInPlace.h"
#include "BlasterAgain/GameMode/BlasterGameMode.h"
#include "BlasterAgain/PlayerController/BlasterPlayerController.h"
#include "BlasterAgain/PlayerState/BlasterPlayerState.h"
#include "BlasterAgain/Weapon/WeaponTypes.h"
#include "Components/CapsuleComponent.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BlasterAgain/BlasterAgain.h"
#include "BlasterAgain/GameState/BlasterGameState.h"
#include "BlasterAgain/PlayerStart/TeamPlayerStart.h"
#include "Components/BoxComponent.h"
#pragma region Init
ABlasterCharacter::ABlasterCharacter()
{

	PrimaryActorTick.bCanEverTick = true;
	
	//创建相机弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	//使用Pawn控制旋转
	CameraBoom->bUsePawnControlRotation = true;

	//创建第一人称前置摄像头
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//摄像机绑到角色头上的骨骼插槽里
	FollowCamera->SetupAttachment(GetMesh(), FName(TEXT("FollowCameraSocket")));

	//创建第三人称摄像机
	TPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPSCamera"));
	TPSCamera->SetupAttachment(CameraBoom);
	
	//bUseControllerRotationYaw = false;
	//GetCharacterMovement()->bOrientRotationToMovement = true;
	
	FollowCamera->bUsePawnControlRotation = true;
	FollowCamera->SetActive(true); //激活第一人称摄像机，其实这个函数里面的实现就是下面Activate(),等开始游戏的时候切换到第一人称
	FollowCamera->Activate();//还是激活，我不知道不写这行只写上面的行不行，不过我看了SetActive没有网络同步的内容，但是Activate()里面有Broadcast广播，所以我觉得还是留着吧，可以测试下
	TPSCamera->SetActive(false); //关闭第三人称摄像机 这样进去后默认看到的是第一人称摄像机
	TPSCamera->Deactivate();

	//头部显示HUD 添加到角色Mesh上
	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverHeadWidget->SetupAttachment(GetMesh());

	//创建战斗组件，如果需要就创建，这个变量可以由游戏模式控制，如果是一个不需要战斗的游戏模式直接不创建这个组件到角色身上，这样可以减小消耗
	if(bUseCombatComp)
	{
		CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
		CombatComp->SetIsReplicated(true);
	}

	//同上选择性创建
	if(bUseBuffComp)
	{
		Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
		Buff->SetIsReplicated(true);
	}

	//同上
	if(bUseLagCompensationComp)
	{
		LagCompensationComp = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComp"));
	}
	
	// 获取当前角色材质插槽的序号 
	int32 MaterialSlotIndex0 = GetMesh()->GetMaterialIndex("18 - Default");//角色下半身材质
	int32 MaterialSlotIndex1 = GetMesh()->GetMaterialIndex("12 - Default");//角色上半身材质
	// 设置为透明的材质
	GetMesh()->SetMaterial(MaterialSlotIndex0, NormalMaterialDown);
	GetMesh()->SetMaterial(MaterialSlotIndex1, NormalMaterialUp);

	GetCharacterMovement()->RotationRate =FRotator(0.f,0.f,850.f);//设置角色转向速率
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;//相当于开启蓝图里角色移动组件的可蹲伏选项
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//设置对相机组件的碰撞忽略
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);//将人物的碰撞类型设置为自定义的宏SkeletalMesh类型
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//设置对相机组件的碰撞忽略
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);//设置对射线检测的碰撞

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;//上来先初始化下旋转方向，让它别动

	NetUpdateFrequency = 66.f;//网络更新频率，每秒66次
	MinNetUpdateFrequency = 33.f;//最小网络更新频率，每秒33次

	//创建时间轴组件
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeLineComponent"));
	//投掷物
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//
	//身体的hitbox,碰撞通道记得改成HitBox
	//
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);


	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);


	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);


	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);


	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);


	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);


	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);


	/*hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("LeftHandSocket"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);


	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("RightHandSocket"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);


	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);


	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);
	*/


	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);


	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);


	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);


	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);


	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);


	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	for(auto Box : HitCollisionBoxes)
	{
		if(Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//先设置所有通道为忽略
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);//单独设置hitbox通道为碰撞
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	/*if (GetNetMode() == NM_DedicatedServer && GetMesh())
	{
		GetMesh()->VisibilityBasedAnimTickOption=   EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}*/
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly); 
	DOREPLIFETIME(ABlasterCharacter, IsRunning)
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);

}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	ServerSetPlayerName(LocalPlayerName);
	ServerSetMaterial();
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	SpawnDefaultWeapon();//生成初始武器，具体哪把需要在蓝图中设置
	UpdateHUDAmmo();//拿到初始武器后设置HUD弹药
	UpdateHUDHealth();//更新生命值HUD
	UpdateHUDShield();//更新护盾HUD
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
	if(AttachedGrenade)//初始化设置手雷的可视性为fasle
		{
		AttachedGrenade->SetVisibility(false);
		}
	///

	//这段要一直放在函数最后
	if(HasAuthority())
	{
		//服务端的本地就是服务端自己的客户端
		ClientChangeView();
		ClientChangeView();
		return;
	}
	//如果是客户端执行，让服务端去改变，再传递给各个客户端来完成同步的效果
	ServerChangeView();
	ServerChangeView();
	
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
	//HideCameraIfCharacterClose();//如果角色和相机的距离过近则隐藏角色和枪械的模型
	PollInit();//初始化BlasterPlayerState
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			OnPlayerStateInitialized();

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(CombatComp)
	{
		CombatComp->Character = this;//在请求初始化组件函数中设定了 CombatComponent组件中Character变量的值是谁.
	}
	if(Buff)
	{
		Buff->Character = this;
	}
	if(LagCompensationComp)
	{
		LagCompensationComp->Character = this;
		if(Controller)
		{
			LagCompensationComp->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}
#pragma endregion Init



void ABlasterCharacter::PlayFireMontage(bool bAiming)//播放开火的蒙太奇动画
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);//播放开火的蒙太奇动画
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");//选定是开镜的蒙太奇还是没开镜的
		AnimInstance->Montage_JumpToSection(SectionName);//直接跳转到指定的蒙太奇动画
	}
}

void ABlasterCharacter::PlayReloadMontage()//播放重装弹夹的动画蒙太奇
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)//检查武器组件是否为空，同时组件中是否存在已经装备的武器
		{
			return;
		}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();//获取角色的模型再获取动画实例
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);//播放装弹的蒙太奇动画
		FName SectionName;
		
		switch (CombatComp->EquippedWeapon->GetWeaponType())//选择武器的种类并播放动画，添加新的武器后，在此处添加代码
		{
		case EWeaponType::EWT_AssaultRifle :
			SectionName = FName("Rifle");
			break;

		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;

		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;

		case EWeaponType::EWT_SMG:
			SectionName = FName("Pistol");
			break;

		case EWeaponType::EWT_ShotGun:
			SectionName = FName("ShotGun");
			break;

		case EWeaponType::EWT_Sniper:
			SectionName = FName("Sniper");
			break;

		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);//直接跳转到制定的蒙太奇动画
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	//直接让玩家自由倒下
	//GetMesh()->SetSimulatePhysics(true);
	//开启重力
	//GetMesh()->SetEnableGravity(true);
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);//播放升天的蒙太奇动画
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);//播放扔手雷的蒙太奇动画
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
}

void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapons();
	MulticastElim(bPlayerLeftGame);
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyedWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::DropOrDestroyWeapons()
{
	if (CombatComp)
	{
		if (CombatComp->EquippedWeapon)
		{
			DropOrDestroyWeapon(CombatComp->EquippedWeapon);
		}
		if (CombatComp->SecondaryWeapon)
		{
			DropOrDestroyWeapon(CombatComp->SecondaryWeapon);
		}
		if (CombatComp->TheFlag)
		{
			CombatComp->TheFlag->Dropped();
		}
	}
}

void ABlasterCharacter::SetSpawnPoint()
{
	if(HasAuthority() && BlasterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor* > PlayerStarts;
		//获取所有的复活点
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		//设置团队复活点
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		//对于所有的复活点
		for(auto Start : PlayerStarts)
		{
			//将普通复活点转为团队复活点类型
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			//复活点要与玩家所在队伍一致
			if(TeamStart && TeamStart->Team == BlasterPlayerState->GetTeam())
			{
				//将转换后的复活点添加到团队复活点组去
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		//如果不止一个玩家复活点的话
		if(TeamPlayerStarts.Num() > 0)
		{
			//要被使用的复活点 = 随机一个复活点
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0,TeamPlayerStarts.Num()-1)];

			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(),
				ChosenPlayerStart->GetActorRotation()
			);
		}

	}
}

void ABlasterCharacter::OnPlayerStateInitialized()
{
	BlasterPlayerState->AddToScore(0.f);
	BlasterPlayerState->AddToDefeats(0);
	SetTeamColor(BlasterPlayerState->GetTeam());//设置队伍颜色
	SetSpawnPoint();
}

void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)//玩家淘汰时
{
	bLeftGame = bPlayerLeftGame;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);//死亡后设置HUD子弹数量为0
	}
	bElimed = true;
	PlayElimMontage();

	//开始溶解效果
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"),0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
		StartDissolve();
	}

	//禁用角色移动
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	/*if (BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}禁用玩家输入*/
	bDisableGameplay = true;//直接将此项设置为true，则禁用相关项的输入响应
	GetCharacterMovement()->DisableMovement();//击杀玩家之后禁用了移动和重力防止玩家掉入虚空
	if(CombatComp)
	{
		CombatComp->FireButtonPressed(false);//如果玩家这个时候有武器且在按开火键就关掉
	}
	//禁用碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);//玩家被淘汰后设置手雷的模型为不碰撞

	//生成淘汰音效
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 150.f);
		ElimBotComponent =  UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
			);
	}
	if (ElimBotSound)
	{//在生成的位置播放淘汰音效
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}

	bool bHideSniperScope = IsLocallyControlled() && CombatComp && CombatComp->bAiming && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper;
	if(bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	//淘汰的时候就销毁ACE特效
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
		
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);

	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && CombatComp && CombatComp->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(CombatComp->CarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(CombatComp->EquippedWeapon->GetAmmo());
	}
}

void ABlasterCharacter::SpawnDefaultWeapon()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	UWorld *World = GetWorld();
	if(BlasterGameMode && World && !bElimed && DefaultWeaponClass)
	{
		AWeapon * StartingWeapon =  World->SpawnActor<AWeapon>(DefaultWeaponClass);
		if(CombatComp)
		{
			CombatComp->EquipWeapon(StartingWeapon);
		}

	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (CombatComp == nullptr)
	{
		return;
	}//会检查被命中的人是否持有武器，若无武器则不会播放受击动画
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
	
		AnimInstance->Montage_Play(HitReactMontage);//播放被击中的蒙太奇动画
		FName SectionName("FromForward");
		//	SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");//选定是开镜的蒙太奇还是没开镜的
		AnimInstance->Montage_JumpToSection(SectionName);//直接跳转到制定的蒙太奇动画
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);//请求离开游戏
	}
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	//如果是空的就生成
	if(CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetMesh(),
			FName(),
			GetActorLocation() + FVector(0, 0, 100.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false//取消自动销毁
		);
	}
	//如果已经生成过了就用下面的
	if(CrownComponent)
	{
		CrownComponent->Activate();//激活粒子系统
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if(CrownComponent)
	{
		//存在粒子组件就销毁
		CrownComponent->DestroyComponent();
	}
}

void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr ) return;

	switch (Team)
	{
	case ETeam::ET_NoTeam :
		GetMesh()->SetMaterial(0,OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;

	case ETeam::ET_BlueTeam :
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;

	case ETeam::ET_RedTeam :
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
	}
}


#pragma region PlayerName
void ABlasterCharacter::ClientSetName_Implementation(const FString& Name)
{
	//设置玩家名称 Set Player Name
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController->PlayerState)
	{
		PlayerController->PlayerState->SetPlayerName(Name);
	}
}

void ABlasterCharacter::ServerSetPlayerName_Implementation(const FString& PlayerName)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		// 在服务端设置玩家名称 Set player name in server
		PlayerController->PlayerState->SetPlayerName(PlayerName);
		// 调用ClientSetPlayerName函数在客户端设置玩家名称 use ClientSetPlayerName to set player name in client 
		ClientSetName(PlayerName);
	}
}
#pragma endregion PlayerName

#pragma region Input

void ABlasterCharacter::MoveForward(const FInputActionValue& InputValue)
{
	float value = InputValue.GetMagnitude();

	if ((Controller != nullptr) && (value != 0.f))
	{
		// 判断哪个旋转方向是向前的
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// 获取向前的向量
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
	}

}

void ABlasterCharacter::MoveRight(const FInputActionValue& InputValue)
{
	float value = InputValue.GetMagnitude();

	if (Controller != nullptr && value != 0.f)
	{
		// 判断哪个旋转方向是向右的
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// 获取向右的向量
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
	}

}

void ABlasterCharacter::LookRightRate(const FInputActionValue& InputValue)
{
	AddControllerYawInput(InputValue.GetMagnitude() * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABlasterCharacter::LookUpRate(const FInputActionValue& InputValue)
{
	AddControllerPitchInput(InputValue.GetMagnitude() * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABlasterCharacter::OnJump(const FInputActionValue& InputValue)
{
	Jump();
	if(bIsCrouched)
	{
		UnCrouch();
	}
}

void ABlasterCharacter::OnJumpStoping(const FInputActionValue& InputValue)
{
	StopJumping();
}


void ABlasterCharacter::RunSpeedWalk(const FInputActionValue& InputValue)
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	ServerRunSpeed();
	IsRunning = true;
}

void ABlasterCharacter::NormalSpeedWalk(const FInputActionValue& InputValue)
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	ServerNormalSpeedWalk();
	IsRunning = false;
}


void ABlasterCharacter::ServerRunSpeed_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	IsRunning = true;
}


void ABlasterCharacter::ServerNormalSpeedWalk_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	IsRunning = false;
}

void ABlasterCharacter::EquipButtonPressed(const FInputActionValue& InputValue)
{
	if(CombatComp)
	{
		if(HasAuthority())
		{
			CombatComp->EquipWeapon(OverlappingWeapon);//如果就在Server上就直接执行
		}
		else
		{
			ServerEquipButtonPressed();//不然就执行RPC  
		}

	}

	
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	//反正只在服务端调用，不用写HasAuthority 
	if (CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::InputDropWeapon()
{
	if (CombatComp)
	{
		if (HasAuthority())
		{
			CombatComp->DropWeapon();//如果就在Server上就直接执行
		}
		else
		{
			ServerDropButtonPressed();//不然就执行RPC  
		}
	}
}

void ABlasterCharacter::ServerDropButtonPressed_Implementation()
{
	//反正只在服务端调用，不用写HasAuthority 
	if (CombatComp)
	{
		CombatComp->DropWeapon();
	}
}

void ABlasterCharacter::CrouchButtonPressed(const FInputActionValue& InputValue)
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::InputAimingPressed(const FInputActionValue& InputValue)
{
	//PlayerController->SetViewTargetWithBlend(CurrentFPSCamera, 0.1f);
	if (CombatComp)
	{
		CombatComp->SetAiming(true);

	}
}

void ABlasterCharacter::InputAimingReleased(const FInputActionValue& InputValue)
{
	if (CombatComp)
	{
		CombatComp->SetAiming(false);
	}
}
#pragma endregion Input

#pragma  region notstart
void ABlasterCharacter::FirePressed(const FInputActionValue& InputValue)
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入
	if (CombatComp && CombatComp->bHoldingTheFlag) return;

	if (CombatComp)
	{
		CombatComp->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireReleased(const FInputActionValue& InputValue)
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入
	if (CombatComp && CombatComp->bHoldingTheFlag) return;

	if (CombatComp)
	{	
		CombatComp->FireButtonPressed(false);
	}
}




void ABlasterCharacter::InputReload(const FInputActionValue& InputValue)
{
	if (bDisableGameplay)return;//如果设置为true，则代表禁用了此项输入
	if (CombatComp && CombatComp->bHoldingTheFlag) return;
	
	if (CombatComp)
	{
		CombatComp->Reload();
	}
}
#pragma  endregion notstart

#pragma region ChangeView
void ABlasterCharacter::InputShiftView(const FInputActionValue& InputValue)
{
	//如果是服务端执行，就相当于调用一次本地客户端，然后直接return
	if(HasAuthority())
	{
		//服务端的本地就是服务端自己的客户端
		ClientChangeView();
		return;
	}
	//如果是客户端执行，让服务端去改变，再传递给各个客户端来完成同步的效果
		ServerChangeView();
}


void ABlasterCharacter::ServerChangeView_Implementation()
{
	//需要修改服务器上的版本，执行到这了，这个函数是在服务端上运行的，ChangeCameraView改变的是服务端上的备份。
	//因为服务器上的版本是用来同步给别的客户端的，别的客户端不与正在执行动作的客户端通信，别的客户端拿的是服务端的数据
		ChangeCameraView();
	//让正在执行动作的客户端去真正实现功能
	
		ClientChangeView();//注释此函数后，客户端角色能同步旋转但是不能改变视角，原因是没有收到Server发来的ClientChangeView()所以改变不了视角，
							//但是和角色移动旋转相关的属性是默认实现网络同步的，是否旋转属性在上面Server端被修改的时候就被同步到客户端了，因此不需要Server发来的ClientChangeView()也改变了旋转。
}

void ABlasterCharacter::ClientChangeView_Implementation()
{
	ChangeCameraView();
}

void ABlasterCharacter::ChangeCameraView()
{
	// 判断当前激活的摄像机，并切换到另一个摄像机
	if (FollowCamera->IsActive())//切换到第三人称
	{
		FollowCamera->SetActive(false);
		FollowCamera->Deactivate();
		TPSCamera->SetActive(true);
		TPSCamera->Activate();
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
		if(IsLocallyControlled())
		{
			// 获取当前材质插槽的数量 
			int32 MaterialSlotIndex0 = GetMesh()->GetMaterialIndex("18 - Default");//下半身
			int32 MaterialSlotIndex1 = GetMesh()->GetMaterialIndex("12 - Default");//上半身
			// 设置为正常的材质
			GetMesh()->SetMaterial(MaterialSlotIndex0, NormalMaterialDown);
			GetMesh()->SetMaterial(MaterialSlotIndex1, NormalMaterialUp);
		}
		
	}
	else//切换到第一人称
	{
		TPSCamera->SetActive(false);
		TPSCamera->Deactivate();
		FollowCamera->SetActive(true);
		FollowCamera->Activate();
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
		if (IsLocallyControlled())
		{
			// 获取当前材质插槽的数量 
			int32 MaterialSlotIndex0 = GetMesh()->GetMaterialIndex("18 - Default");//下半身
			int32 MaterialSlotIndex1 = GetMesh()->GetMaterialIndex("12 - Default");//上半身
			// 设置为透明的材质
			GetMesh()->SetMaterial(MaterialSlotIndex0, TransparentMaterial);
			GetMesh()->SetMaterial(MaterialSlotIndex1, TransparentMaterial);
		}
	}
}


void ABlasterCharacter::ClientSetMaterial_Implementation()
{
	// 获取当前材质插槽的数量 
	int32 MaterialSlotIndex0 = GetMesh()->GetMaterialIndex("18 - Default");//下半身
	int32 MaterialSlotIndex1 = GetMesh()->GetMaterialIndex("12 - Default");//上半身
	if (IsLocallyControlled())//如果是本机就本地设为透明
	{
		GetMesh()->SetMaterial(MaterialSlotIndex0, TransparentMaterial);
		GetMesh()->SetMaterial(MaterialSlotIndex1, TransparentMaterial);
	}
	else//如果不是本机
	{
		// 设置为正常的材质
		GetMesh()->SetMaterial(MaterialSlotIndex0, NormalMaterialDown);
		GetMesh()->SetMaterial(MaterialSlotIndex1, NormalMaterialUp);
	}
}

void ABlasterCharacter::ServerSetMaterial_Implementation()
{
	//别人看都正常，别人看不能是透明的
	// 获取当前材质插槽的数量
		// 获取当前材质插槽的数量 
	int32 MaterialSlotIndex0 = GetMesh()->GetMaterialIndex("18 - Default");//下半身
	int32 MaterialSlotIndex1 = GetMesh()->GetMaterialIndex("12 - Default");//上半身

	if (IsLocallyControlled())//如果是本机就本地设为透明，但下面的Client还是正常
	{
		GetMesh()->SetMaterial(MaterialSlotIndex0, TransparentMaterial);
		GetMesh()->SetMaterial(MaterialSlotIndex1, TransparentMaterial);
	}
	else
	{
		// 设置为正常的材质
		GetMesh()->SetMaterial(MaterialSlotIndex0, NormalMaterialDown);
		GetMesh()->SetMaterial(MaterialSlotIndex1, NormalMaterialUp);
	}
	ClientSetMaterial();

}

#pragma endregion ChangeView


#pragma region ShowOrHidePickupWidget

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	OverlappingWeapon = Weapon;//如果是客户端，不会执行这个函数，因为是无主的weapon调用的这个函数，客户端触发碰撞后会在服务端执行，然后OnRep会把OverlappingWeapon传到其他客户端去，又因为被定义为COND_OwnerOnly，所以只有Owner对应的客户端拿到了值，OnRep由服务端调用，不在服务端执行只在客户端执行，所以Server不会触发OnRep函数
	//因为Server不会触发OnRep函数，所以若是Server的玩家触发的碰撞，通过下面的方法进行显示。
	//只有Server上的本地玩家执行，加上函数本身是由在服务端AWeapon::OnSphereOverlap调用的
	if(IsLocallyControlled())//Returns true if controlled by a local (not network) Controller.实测注释这个函数没有影响，因为调用这个函数的对象的特殊性确保了代码运行到这的时候只可能是服务端在运行，客户端压根不会调用这个函数，这里参照教程保留这个函数。特殊性为，武器是无主的，无主的都归server
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
		
	}
}


void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())return;
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)//当角色和相机组件的距离过近时，让角色在本地消失从而不影响视野
		{
		GetMesh()->SetVisibility(false);//设置人物不可见	
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;//设置拥有者不可见
		}
		if (CombatComp && CombatComp->SecondaryWeapon && CombatComp->SecondaryWeapon->GetWeaponMesh())
		{
			CombatComp->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;//设置拥有者不可见
		}
		}
	else
	{
		GetMesh()->SetVisibility(true);//设置人物可见	
		if (CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;//设置拥有者可见
		}
		if (CombatComp && CombatComp->SecondaryWeapon && CombatComp->SecondaryWeapon->GetWeaponMesh())
		{
			CombatComp->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;//设置拥有者可见
		}
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector  Velocity = GetVelocity();
	Velocity.Z = 0.f;

	return Velocity.Size(); //获取三维向量的模长，其中Z为0
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if(Health < LastHealth)//现在的血量比之前的低，说明受到了伤害
		{
			PlayHitReactMontage();
		}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)//现在的护盾比之前的低，说明受到了伤害
		{
			PlayHitReactMontage();
		}
}

void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;

	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);//请求复活玩家
	}
	if(bLeftGame &&IsLocallyControlled() )//如果变量
		{
			OnLeftGame.Broadcast();
		}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

#pragma endregion ShowOrHidePickupWidget

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if(CombatComp && CombatComp->EquippedWeapon == nullptr) return;//如果没这个组件或者手上没武器 是不需要执行这个函数的
	AO_Pitch = GetBaseAimRotation().Pitch;//控制持枪动画的上下，不管是第一人称还是第三人称都需要
	if(AO_Pitch > 90.f && !IsLocallyControlled())//在网络传输过程中，Pitch会从float压缩成unsigned进行传输从而丢失正负，需要进行转换
	{
		//将AO_Pitch从[270,360)映射到[-90,0];
		FVector2D InRange(270.f,360.f);
		FVector2d OutRange(-90.f,0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange,OutRange,AO_Pitch);
	}
	if(IsFirstPerson())//第一人称枪在手，Yaw随鼠标走
	{
		bUseControllerRotationYaw = true;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0;
		return;
		//无需处理后面的
	}
	
		
		//处理第三人称下的偏移
		FVector Velocity = GetVelocity();
		Velocity.Z = 0.f;
		//设置速度
		float Speed = Velocity.Size();
		//是否在空中
		bool bIsInAir = GetCharacterMovement()->IsFalling();

		if(Speed == 0.f && !bIsInAir)//静止状态没有跳跃
			{
			FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
			FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation,StartingAimRotation);
			AO_Yaw = DeltaAimRotation.Yaw;
			if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
			{
				InterpAO_Yaw = AO_Yaw;
			}
			bUseControllerRotationYaw = false;//关闭角色朝向旋转
			}
		if(Speed >0.f || bIsInAir)//奔跑或者跳跃
			{
			AO_Yaw = 0.f;	
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
			bUseControllerRotationYaw = true;
			}
	
	
}



void ABlasterCharacter::SimProxiesTurn()
{
	if (CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr) return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw =  UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;//计算上一帧到现在的旋转差值
	
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)//修改模拟代理角色的朝向
			{
			TurningInPlace = ETurningInPlace::ETIP_Right;
			}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.F)//向右旋转角度超过90度
		{
		TurningInPlace = ETurningInPlace::ETIP_Right;
		}

	else if (AO_Yaw < -90.F)//向左旋转角度超过90度
		{
		TurningInPlace = ETurningInPlace::ETIP_Left;
		}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);//插值缓入
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)//旋转度数不大的话
			{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);//每帧存储Yaw

			}
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	BlasterGameMode = BlasterGameMode == nullptr  ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	if(bElimed || BlasterGameMode == nullptr) return;//如果自己已经死了就不会再受到伤害了

	Damage = BlasterGameMode->CalculateDamage(InstigatorController, Controller, Damage , bTeamDamage , TeamDamageRate);

	float DamageToHealth = Damage;
	float NotHitedShield = 0.f;
	if(Shield>0)
	{
		if(DamageToHealth > Shield)//伤害比护盾高，破防扣血
			{
			NotHitedShield = Shield;//存储一次还没收到攻击时的护盾值
			Shield = FMath::Clamp(Shield - DamageToHealth, 0.f, MaxShield);//设置新的护盾值和受到的伤害
			DamageToHealth = DamageToHealth - NotHitedShield;//设置对血条造成的伤害为扣除护盾抵挡后的伤害
			//播放被击中身体的声音
			}
		else//伤害比护盾低，只有护盾降低
			{
			Shield = FMath::Clamp(Shield - DamageToHealth, 0.f, MaxShield);//设置护盾值和受到的伤害
			DamageToHealth = 0.f;//本次攻击不会对血条造成影响
			//播放被击中护盾的声音
			}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);//设置生命值和受到的伤害
	UpdateHUDShield();
	UpdateHUDHealth();

	PlayHitReactMontage();
	if (Health == 0.f)//如果血量到0,进行淘汰
		{
		if (BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController );
		}
		}
}



bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatComp && CombatComp->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (CombatComp && CombatComp->bAiming);
}

bool ABlasterCharacter::IsFirstPerson()
{
	return FollowCamera->IsActive();
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (CombatComp == nullptr) return nullptr;
	return CombatComp->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (CombatComp == nullptr) return FVector();
	return CombatComp->HitTarget;
}


ECombatState ABlasterCharacter::GetCombatState() const
{
	if (CombatComp == nullptr) return ECombatState::ECS_Max;
	return CombatComp->CombatState;
}

bool ABlasterCharacter::IsHoldingTheFlag()
{
	if (CombatComp == nullptr) return false;
	return CombatComp->bHoldingTheFlag;
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if (CombatComp == nullptr) return false;
	return CombatComp->bLocallyReloading;
}

ETeam ABlasterCharacter::GetTeam()
{
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	if (BlasterPlayerState == nullptr) return ETeam::ET_NoTeam;
	return BlasterPlayerState->GetTeam();
}

void ABlasterCharacter::SetHoldingTheFlag(bool bHolding)
{
	if (CombatComp == nullptr) return;
	CombatComp->bHoldingTheFlag = bHolding;
}


void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//使用控制器控制旋转pitch和yaw
	//bUseControllerRotationPitch = true;
	//bUseControllerRotationYaw = true;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{

			//先清除映射避免重复添加
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(IMC_Action, 0);
			Subsystem->AddMappingContext(IMC_MoveBase, 0);
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent)
	{

		//移动
		EnhancedInputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveForward);
		EnhancedInputComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveRight);
		//旋转
		EnhancedInputComponent->BindAction(IA_LookRightRate, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookRightRate);
		EnhancedInputComponent->BindAction(IA_LookUpRate, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookUpRate);
		//跳跃
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ABlasterCharacter::OnJump);

		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ABlasterCharacter::OnJumpStoping);

		//射击
		EnhancedInputComponent->BindAction(IA_FirePressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::FirePressed);

		EnhancedInputComponent->BindAction(IA_FireReleased, ETriggerEvent::Triggered, this, &ABlasterCharacter::FireReleased);

		//右键瞄准
		EnhancedInputComponent->BindAction(IA_AimingPressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputAimingPressed);

		EnhancedInputComponent->BindAction(IA_AimingReleased, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputAimingReleased);
		//shift奔跑
		EnhancedInputComponent->BindAction(IA_ShiftPressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::RunSpeedWalk);

		EnhancedInputComponent->BindAction(IA_ShiftReleased, ETriggerEvent::Triggered, this, &ABlasterCharacter::NormalSpeedWalk);

		//按下Ctrl下蹲
		EnhancedInputComponent->BindAction(IA_CrouchPressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::CrouchButtonPressed);

		//按G丢弃武器
		EnhancedInputComponent->BindAction(IA_DropWeapon, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputDropWeapon);

		//按R武器换弹
		EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputReload);

		//按V切换视角
		EnhancedInputComponent->BindAction(IA_ChangeView, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputShiftView);

		//按F拾取武器
		EnhancedInputComponent->BindAction(IA_Equip, ETriggerEvent::Triggered, this, &ABlasterCharacter::EquipButtonPressed);

		
		// 你可以通过更改"ETriggerEvent"枚举值，绑定到此处的任意触发器事件.
		//Input->BindAction(AimingInputAction, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::SomeCallbackFunc);

	}
}



