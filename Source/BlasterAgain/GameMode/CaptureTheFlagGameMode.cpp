// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"

#include "BlasterAgain/BlasterTypes/Team.h"
#include "BlasterAgain/GameState/BlasterGameState.h"
#include "BlasterAgain/PlayerStart/FlagZone.h"
#include "BlasterAgain/Weapon/Flag.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter,
                                               ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	//直接调用父父类的,击杀淘汰玩家
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);


}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(GameState);
	if(BGameState)
	{
		//蓝队加分
		if(Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
		//红队加分
		if(Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
	}
}