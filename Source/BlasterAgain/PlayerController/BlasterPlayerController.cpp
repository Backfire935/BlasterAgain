// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"

#include "BlasterAgain/BlasterComponent/CombatComponent.h"
#include "BlasterAgain/BlasterTypes/Announcement.h"
#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/GameMode/BlasterGameMode.h"
#include "BlasterAgain/GameMode/TeamsGameMode.h"
#include "BlasterAgain/GameState/BlasterGameState.h"
#include "BlasterAgain/HUD/Announcement.h"
#include "BlasterAgain/HUD/BlasterHUD.h"
#include "BlasterAgain/HUD/CharacterOverlay.h"
#include "BlasterAgain/HUD/ReturnToMainMenu.h"
#include "BlasterAgain/PlayerState/BlasterPlayerState.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)//由客户端发起的，传入的是客户端发送请求时的时间，要求获取服务端的时间
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();//服务端接收到客户端请求时候的时间
	ClientReportServerTime(TimeOfClientRequest,ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)//由服务端发起的，将服务端当前的时间发送给客户端，来回应客户端发起的ServerRequestServerTime请求
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;//将服务端的当前时间减去客户端发送请求时候的时间，得到网络传递信息的延迟时间。
	SingleTripTime = 0.5 * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;//服务端接收到客户端请求时候的时间 + 0.5*网络传递信息的延迟时间 = 与客户端同步的服务端的真正时间
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();//客户端和服务端的时间差异

}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController, MatchState);
		DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores);

}

void ABlasterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);

}

void ABlasterPlayerController::HideTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScore &&
		BlasterHUD->CharacterOverlay->RedTeamScore &&
		BlasterHUD->CharacterOverlay->TargetScore;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->TargetScore->SetText(FText());

	}
}

void ABlasterPlayerController::InitTeamScores()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScore &&
		BlasterHUD->CharacterOverlay->RedTeamScore &&
		BlasterHUD->CharacterOverlay->TargetScore;

		TeamGameMode = TeamGameMode == nullptr ? GetWorld()->GetAuthGameMode<ATeamsGameMode>() : TeamGameMode;

	if (bHUDValid)
	{
		float Score = TeamGameMode->GetTargetScore();
		FString Zero("0");
		FString Target = FString::Printf(TEXT("%.0f"), Score);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->TargetScore->SetText(FText::FromString(Target));

	}

}

void ABlasterPlayerController::SetHUDRedTeamScores(int32 RedScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		 TeamGameMode = TeamGameMode == nullptr ? GetWorld()->GetAuthGameMode<ATeamsGameMode>() : TeamGameMode;
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));

	}
}

void ABlasterPlayerController::SetHUDBlueTeamScores(int32 BlueScore)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScore;
	if (bHUDValid)
	{
		TeamGameMode = TeamGameMode == nullptr ? GetWorld()->GetAuthGameMode<ATeamsGameMode>() : TeamGameMode;
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));

	}
}

void ABlasterPlayerController::SetHUDTargetScore(int32 Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->TargetScore;
	if (bHUDValid)
	{
		TeamGameMode = TeamGameMode == nullptr ? GetWorld()->GetAuthGameMode<ATeamsGameMode>() : TeamGameMode;
		FString ScoreText = FString::Printf(TEXT("%d"), Score);
		BlasterHUD->CharacterOverlay->TargetScore->SetText(FText::FromString(ScoreText));

	}
}

void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if(Attacker && Victim && Self)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if(BlasterHUD)
		{
			//我杀了别人
			if(Attacker == Self && Victim != Self)
			{
				BlasterHUD->AddElimAnnouncement("你", Victim->GetPlayerName());
				return;
			}
			//别人杀了我
			if(Victim == Self && Attacker != Self)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "你");
				return;
			}
			//自己把自己干掉了
			if(Attacker == Victim && Attacker == Self)
			{
				BlasterHUD->AddElimAnnouncement("你", "你自己");
				return;
			}
			//别人自杀了
			if (Attacker == Victim && Attacker != Self)
			{
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "自己");
				return;
			}
			//别人杀了别人
			BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(),Victim->GetPlayerName());
		}
	}
}


void ABlasterPlayerController::ShowReturnToMainMenu()
{
	//显示回到主菜单的界面
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if(ReturnToMainMenu)
	{
		bReturnToMenuOpen = !bReturnToMenuOpen;
		if(bReturnToMenuOpen)
		{
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

void ABlasterPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = 	Cast<ABlasterHUD>(GetHUD());

	ServerCheckMatchState();
	
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime); //检查时间同步的函数
	PollInit();//检查HUD类是否创建并拿到了值
	CheckPing(DeltaTime);

}

void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			float Ping = 0;
			Ping = PlayerState->GetPingInMilliseconds();

			BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

			UE_LOG(LogTemp, Warning, TEXT("%.0f"), Ping);
			bool bValid = BlasterHUD &&
				BlasterHUD->CharacterOverlay &&
				BlasterHUD->CharacterOverlay->PingText;
			if(bValid)
			{
				FString Text = FString::Printf(TEXT("%.0fms"), Ping);//将ping从float转为FString
				BlasterHUD->CharacterOverlay->PingText->SetText(FText::FromString(Text));//将ping从FString转为FText
			}
	
			
			if (Ping > HighPingThreshold)//得到ping值并和警告的阈值进行比较
			{
				HighPingWarning(Ping);
				PingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);//传递延迟状态确定，以便后续确定是否启用ssr
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0;//计时器清零
	}
	bool bHighPingAnimationPlaying =
		BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->HighPingAnimation
		&& BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)//如果在播放高ping动画，就停止播放
	{
		PingAnimationRunningTime += DeltaTime;//动画播放时间的计时
		if (PingAnimationRunningTime > HighPingDuration)//超过设置的时间
		{
			StopHighPingWarning();//停止播放
		}
	}
}

//判断ping是否过高
void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;//改变达到检查同步要求的时间
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());//请求同步时间
		TimeSyncRunningTime = 0.f;//计时器置零
	}
}

void ABlasterPlayerController::SetHUDTime()//控制设置HUD,此函数在类Tick中调用
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)  TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if(MatchState == MatchState::Cooldown)TimeLeft =CooldownTime+ WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;

	//uint32 SecondsLeft = FMath::CeilToInt( MatchTime - GetServerTime());//游戏总时间-游戏开始的时间=游戏剩下的时间，CeilToInt函数内虽然是两个浮点型变量在每帧相减，但是在转换成uInt后，只有发生整数的变化时，下面的if判断才会生效
	uint32 SecondsLeft = FMath::CeilToInt( TimeLeft);


	/*	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if(BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
		}*/
	

	if (CountdownInt != SecondsLeft)//每帧都更新HUD会增大开销，这里判断SecondsLeft是否发生变化，发生了变化才改变HUD
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);//还没开始比赛或者已经结束比赛则展示提示界面
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);//已经开始游戏则展示玩家HUD界面
		}
		//SetHUDMatchCountDown(MatchTime - GetServerTime());//刷新一次HUD
	}

	CountdownInt = SecondsLeft;//运行到这说明SecondsLeft值发生了改变，这个时候也更新下CountdownInt的值
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)//如果HUD没来得及创建或者创建失败
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)//tick中更新一次
			{
				if(bInitializeHealth) SetHUDHealth(HUDHealth,HUDMaxHealth);
				if(bInitializeShield) SetHUDShield(HUDShield,HUDMaxShield);
				if(bInitializeScore) SetHUDScore(HUDScore);
				if(bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if(bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if(BlasterCharacter && BlasterCharacter->GetCombat())
				{
					if(bInitializeGrenades)	SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;
	//绑定Esc到退出界面上
	InputComponent->BindAction("Esc",IE_Pressed, this, &ABlasterPlayerController::ShowReturnToMainMenu);
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(),BlasterCharacter->GetMaxHealth());
		SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());
	}
}

void ABlasterPlayerController::SetElimText(bool bElim)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ElimText;
	if (bHUDValid)
	{
		if(bElim)
		BlasterHUD->CharacterOverlay->HealthText->SetVisibility(ESlateVisibility::Visible);
		else
		BlasterHUD->CharacterOverlay->HealthText->SetVisibility(ESlateVisibility::Hidden);
	}
}



void ABlasterPlayerController::ReceivedPlayer()//在接收玩家的时候就执行GetServerTime，目的是尽快与服务端时间保持一致
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

float ABlasterPlayerController::GetServerTime()//和服务器同步时间
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else  return GetWorld()->GetTimeSeconds() + ClientServerDelta;//本地当前时间+与服务器不同步的时间 = 和服务器同步的时间
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD && 
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar && 
		BlasterHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;//设置血量百分比
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"),FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));//计算并转换当前血量值的文本
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));//设置血量值
	}
	else//如果控件初始化失败，还有个专门用来初始化的函数pollinit
	{
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;

	}
}

void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldBar &&
		BlasterHUD->CharacterOverlay->ShieldText;
	if (bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;//设置护盾百分比
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));//计算并转换当前护盾值的文本
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));//设置护盾值
	}
	else//如果控件初始化失败，还有个专门用来初始化的函数pollinit
	{
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;

	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;
	
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"),FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else//如果控件初始化失败，还有个专门用来初始化的函数pollinit
	{
		bInitializeScore = true;
		HUDScore = Score;
	}

}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;

	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText)); 

	}
	else//如果控件初始化失败，还有个专门用来初始化的函数pollinit
	{
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

void ABlasterPlayerController::SetHUDMatchCountDown(float CountdownTime)//CountdownTime是一局游戏的限制时间
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountDownText;

	if (bHUDValid)
	{

		if(CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountDownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);//总时长的秒数，除以60就是分，算完取int
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		if(CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);//总时长的秒数，除以60就是分，算完取int
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadesText;

	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		HUDGrenades = Grenades;
		bInitializeGrenades = true;
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State, bool bTeamMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime,LevelStartingTime);
	}

}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match,float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)//如果玩家不是在热身阶段进入游戏的话，就不调出这个界面了
	{
		BlasterHUD->AddAnnouncement();//玩家控制器在游戏已开始就调出屏幕提示界面
	}
}

void ABlasterPlayerController::OnRep_MatchState()//OnMatchStateSet函数先被游戏模式调用后更新了MatchState的值，然后再触发这个函数，所以此时MatchState已经更新过了，直接传入服务端的MatchState值给客户端即可
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();//调用客户端上的HandleMatchHasStart函数
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamMatch)
{
	if(HasAuthority()) 
		bShowTeamScores = bTeamMatch;


	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if(BlasterHUD->CharacterOverlay == nullptr) BlasterHUD->AddCharacterOverlay();//开局热身倒计时结束后调用HUD类的显示界面,如果已经有了就不添加了
		if (BlasterHUD->Announcement)//进入正式游戏后隐藏Announcement界面
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	
		//如果不是在服务器上就不执行后面的
		if (!HasAuthority()) return;
		if(bTeamMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

void ABlasterPlayerController::HandleCooldown()//处理游戏结束
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();//比赛结束后关掉角色类的HUD
		bool bHUDValid = BlasterHUD->Announcement
			&& BlasterHUD->Announcement->AnnouncementText
			&& BlasterHUD->Announcement->InfoText;
		if (bHUDValid)//进入正式游戏后隐藏Announcement界面
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));//将提示信息栏文本改为新的宣告信息

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//获取游戏状态
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();//获取自己的游戏状态
			if(BlasterGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;//从游戏状态中获取玩家状态
				FString InfoTextString =  bShowTeamScores ? GetTeamsInfoText(BlasterGameState)  : GetInfoText(TopPlayers);
			/*	FString bs = FString::Printf(TEXT("%hhd + bool"), bShowTeamScores);
				UE_LOG(LogTemp, Warning, TEXT("%s"), *bs);
				UE_LOG(LogTemp, Warning, TEXT("The boolean value is %s"), (bShowTeamScores ? TEXT("true") : TEXT("false")));*/
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}

		}
	}
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if(BlasterCharacter && BlasterCharacter->GetCombat())
	{
		BlasterCharacter->bDisableGameplay = true;//禁用一些玩家输入
		BlasterCharacter->GetCombat()->FireButtonPressed(false);//若之前玩家在手持武器开火，此时会停止开火
	}
}


FString ABlasterPlayerController::GetInfoText(const TArray<class ABlasterPlayerState*>& Players)
{
	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();//获取自己的游戏状态
	if (BlasterPlayerState == nullptr) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)//如果数组大小为0，则说明还没有人拿到分数
	{
		InfoTextString = Announcement::ThereIsNoWinner;
		//InfoTextString = FString("There is no winner.");
	}
	else if (Players.Num() == 1 && Players[0] == BlasterPlayerState)//有人拿到了第一而且这个人是自己
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)//有人拿到了第一但不是自己
	{
		InfoTextString = FString::Printf(TEXT("赢家是:\n%s"), *Players[0]->GetPlayerName());//获取该玩家的名称
	}
	else if (Players.Num() > 1)//不止一人获得了第一的得分
	{
		InfoTextString = Announcement::ManyWinner;

		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));//将所有的优胜者的名字写进去
		}
	}
	return InfoTextString;
}

FString ABlasterPlayerController::GetTeamsInfoText( ABlasterGameState* BlasterGameState)
{
	if (BlasterGameState == nullptr) return FString();
	FString InfoTextString;

	const int32 RedTeamScore = BlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = BlasterGameState->BlueTeamScore;

	if(RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}

	else if(RedTeamScore == BlueTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"),*Announcement::TeamsTiedForTheWin);
		//InfoTextString.Append(Announcement::RedTeam+"\n");
		//InfoTextString.Append(Announcement::BlueTeam+"\n");
	}
	else if(RedTeamScore > BlueTeamScore)
	{
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d分\n"),*Announcement::RedTeam,RedTeamScore ));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d分\n"), *Announcement::BlueTeam, BlueTeamScore));

	}
	else if(BlueTeamScore > RedTeamScore)
	{
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d分\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d分\n"), *Announcement::RedTeam, RedTeamScore));

	}

	return InfoTextString;
}

void ABlasterPlayerController::HighPingWarning(float ping)//播放高ping警告动画
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation&&
		BlasterHUD->CharacterOverlay->PingText;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.F);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation,
				0.f,
			5
			);
		
	}
}


void ABlasterPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation&&
		BlasterHUD->CharacterOverlay->PingText;
	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.F);
		if(BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);//停止播放
		}
		BlasterHUD->CharacterOverlay->PingText->SetText(FText());//置空
	}

}