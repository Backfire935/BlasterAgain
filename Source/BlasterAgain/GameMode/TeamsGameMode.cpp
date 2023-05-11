// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"

#include "BlasterAgain//Character/BlasterCharacter.h"
#include "BlasterAgain/GameState/BlasterGameState.h"
#include "BlasterAgain/PlayerController/BlasterPlayerController.h"
#include "BlasterAgain/PlayerState/BlasterPlayerState.h"
#include"Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamsMatch = true;
	
}

void ATeamsGameMode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATeamsGameMode, TargetScore);
}

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//返回当前游戏状态,世界上下文
	if (BGameState)
	{
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			//最简单的基于人数的匹配队伍机制
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//返回当前游戏状态,世界上下文
	ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if(BGameState && BPState)
	{
		if(BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}

}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage, bool bTeamDamage , float TeamDamageRate)
{
	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (AttackerPState == VictimPState) return BaseDamage;
	//下面是针对相同队伍的情况,禁止友伤
	if(AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		//如果开了友伤
		if(bTeamDamage)
		{
			//限制下伤害倍率的范围
			if (TeamDamageRate > 1) TeamDamageRate = 1;
			// 回血 职业 治疗
			if (TeamDamageRate < 0) TeamDamageRate = 0;
			return BaseDamage * TeamDamageRate;
		}
		else
		{
			//没开友伤就返回零伤害
			return 0.f;
		}
	}
	return  BaseDamage;

}

void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController,
	ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//返回当前游戏状态,世界上下文
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;//AttackerController是否存在，存在就取它的玩家状态，不存在就设空
	ABlasterPlayerState* ElimmedPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;//VictimController是否存在，存在就取它的玩家状态，不存在就设空

	if(BGameState && AttackerPlayerState && ElimmedPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ElimmedPlayerState->GetTeam())
		{//如果是自杀或者杀队友不加分,此条放在最上面
			return;
		}
		if(AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{//淘汰一个红方玩家后蓝队加一分
			BGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{//淘汰一个蓝方玩家后红队加一分
			BGameState->RedTeamScores();
		}
		
	}
}




void ATeamsGameMode::OnRep_TargetScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDTargetScore(TargetScore);
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));//返回当前游戏状态,世界上下文
	if(BGameState)
	{
		//获取所有玩家的玩家状态
		for(auto PState : BGameState->PlayerArray)
		{
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());//UE5新出的的TObject指针，用get方法获取
			if(BPState &&  BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				//最简单的基于人数的匹配队伍机制
				if(BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}

}
