// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"

#include "BlasterAgain/PlayerController/BlasterPlayerController.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
	
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;//推迟游戏开始的进程，使游戏状态为WaitingToStart State,游戏模式会为所有的玩家生成一个默认的看不见的pawn，玩家可以使用这个pawn飞遍整个场景,直到调用StartMatch函数，这个时候游戏模式会生成指定的pawn

}

void ABlasterGameMode::Tick(float Deltatime)
{
	Super::Tick(Deltatime);
	
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;//从打开程序到开始局内游戏的时间-打开程序到现在的时间+热身倒计时时间 = 游戏开始倒计时还剩的时间
		if (CountdownTime <= 0.f)
		{
			StartMatch();//若无意外情况则发生SetMatchState(MatchState::InProgress);，进入游戏状态
		}
	}
	else if(MatchState ==MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;//热身时间+设置的游戏时长-游戏从程序运行到现在的时间+游戏从程序运行到游戏开始的时间 = 距离游戏结束还剩的时间
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime +  WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;//游戏结算的时间+热身时间+设置的游戏时长-游戏从程序运行到现在的时间+游戏从程序运行到游戏开始的时间 = 游戏结算倒计时还剩的时间
		if(CountdownTime <=0.f)
		{
			RestartGame();//游戏结算完毕，重启游戏对局
		}
	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();//从打开程序到开始局内游戏的时间
}

void ABlasterGameMode::OnMatchStateSet()//由游戏模式通知所有的玩家控制器现在的游戏状态
{
	Super::OnMatchStateSet();
	//这是个迭代器
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)	 //允许我们循环遍历所有的玩家控制器
	{
		ABlasterPlayerController *BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState,bTeamsMatch);
		}

	}
}

void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;
		for(auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		//AWeapon* Weapon =  Cast<ABlasterCharacter>(AttackerPlayerState->GetPlayerController()->GetCharacter())->GetEquippedWeapon();

		AttackerPlayerState->AddToScore(1.f);//加一次击杀计数
		BlasterGameState->UpdateTopScore(AttackerPlayerState);//如果攻击者拿到了最高分就会被设置为分数最高的人
		if(BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if(Leader)
			{
				Leader->MulticastGainedTheLead();//拿到第一，给他第一的标
			}

		}

		for(int32 i=0; i< PlayersCurrentlyInTheLead.Num(); i++)
		{
			if(!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ABlasterCharacter* Loser = Cast<ABlasterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				Loser->MulticastLostTheLead();//不是第一了，把特效jue了
			}
		}


	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);//加一次死亡计数

	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It);
		if(BlasterPlayerController && AttackerPlayerState && VictimPlayerState )
		{
			BlasterPlayerController->BroadcastElim(AttackerPlayerState,VictimPlayerState);
		}
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();//从控制器分离并挂起销毁
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart( ElimmedController, PlayerStarts[Selection]);//在玩家开始点重启玩家
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	//调用淘汰玩家函数，为bLeftGame变量传递真值
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ABlasterCharacter * CharacterLeaving =  Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());

}

float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage, bool bTeamDamage, float TeamDamageRate)
{
	return BaseDamage; 
}