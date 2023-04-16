// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	//��Ϸ״̬ ������ÿ����ҵ����״̬ ��һ��TArray
	int32  NumberOfPlayers = GameState.Get()->PlayerArray.Num();//��ҵ�����
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