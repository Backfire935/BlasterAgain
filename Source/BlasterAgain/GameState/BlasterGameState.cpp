// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "BlasterAgain/PlayerController/BlasterPlayerController.h"
#include "BlasterAgain/PlayerState/BlasterPlayerState.h"
#include "Net/UnrealNetwork.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterGameState,TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);

}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)//纪录得到分数的玩家
{
	if(TopScoringPlayers.Num() == 0 )
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();

	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);//确保输入的信息是唯一的，不会输入重复的信息

	}
	else if(ScoringPlayer->GetScore() > TopScore)//如果这个玩家的分比之前的最高分高
		{
		TopScoringPlayers.Empty();//清空之前纪录的最高分
		TopScoringPlayers.AddUnique(ScoringPlayer);//换成新的最高分
		TopScore = ScoringPlayer->GetScore();
		}
}

void ABlasterGameState::RedTeamScores()
{
	++RedTeamScore;
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if(BPlayer)
	{
		BPlayer->SetHUDRedTeamScores(RedTeamScore);
	}

}

void ABlasterGameState::BlueTeamScores()
{
	++BlueTeamScore;
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScores(BlueTeamScore);
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDRedTeamScores(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		BPlayer->SetHUDBlueTeamScores(BlueTeamScore);
	}
}