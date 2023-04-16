// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//游戏状态 包含了每个玩家的玩家状态 是一个TArray
	int32  NumberOfPlayers = GameState.Get()->PlayerArray.Num();//玩家的数量
	if(NumberOfPlayers == 2)
	{
		UWorld* World = GetWorld();
		if(World)
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString("/Game/Map/TestMap?listen"));
		}
	}
}
