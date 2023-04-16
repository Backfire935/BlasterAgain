// Fill out your copyright notice in the Description page of Project Settings.


#include <Menu.h>
#include "Components/Button.h"
#include "MultiplayerSessionSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h" 
void UMenu::MenuSetup(int32 NumberOfPublicConnections , FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen") , *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	//添加到视口
	AddToViewport();
	//设置是否可视
	SetVisibility(ESlateVisibility::Visible);
	//设置是否聚焦界面
	bIsFocusable = true; 
	//获取玩家的键盘鼠标输入信息 就要先创建一个世界
	UWorld* World = GetWorld();
	if (World)//建立成功的话
	{
		//创建玩家控制器
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)//创建成功的话
		{
			
			FInputModeUIOnly InputModeData;//只能从UI界面输入
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true); //显示鼠标指针


		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem =  GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)//绑定委托和回调
	{
		//动态多播委托
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		//静态多播委托
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		//动态多播委托
		MultiplayerSessionsSubsystem ->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);

	}

}
	
bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddDynamic(this, &ThisClass::ExitButtonClicked);
	}
	return true;
}

void UMenu::NativeDestruct()
{

	MenuTearDown();
	
	Super::NativeDestruct();
	//if (InLevel == nullptr && InWorld == GetWorld())
	//{
	//	RemoveFromParent();
	//}
	
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{

	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("Session Create Successfuly"))
			);
		}
		//当会话创建成功的时候再来创建大厅
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Session Create Failed"))
			);
		}
		HostButton->SetIsEnabled(true);
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{

	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
	
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	//	可以通过IOnlineSubsystem访问会话接口
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();

	if (Subsystem)
	{
		//这是一个IOnlineSessionPtr 来接受
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession,Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}

		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);

	}

}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	
	}

}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (GEngine)
		if (MultiplayerSessionsSubsystem)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString(TEXT("Join Button Clicked"))
			);
			MultiplayerSessionsSubsystem->FindSessions(10000);
		}

}

void UMenu::ExitButtonClicked()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString(TEXT("Exit Button Clicked"))
		);
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;//设置仅游戏内操控
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
			
			
		}
	}

}
