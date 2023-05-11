// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"
#include  "GameFrameWork/PlayerController.h"
#include "MultiplayerSessionSubsystem.h"
#include "BlasterAgain/Character/BlasterCharacter.h"
#include "Components/Button.h"
#include"GameFrameWork/GameModeBase.h"

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	
	bIsFocusable = true;
	UWorld* World = GetWorld();
	if(World)
	{
		PlayerController = PlayerController == nullptr ?  World->GetFirstPlayerController() : PlayerController;
		if(PlayerController)
		{
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	if (ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);//用委托来绑定函数和点击事件
	}

	UGameInstance* GameInstance = GetGameInstance();

	if(GameInstance)
	{
		MultiplayerSessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionSubsystem>();
		if(MultiplayerSessionSubsystem)
		{
			MultiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);//会话插件那边销毁会话完成后的回调函数
		}
	}
	

}

void UReturnToMainMenu::MenuTearDown()
{
	RemoveFromParent();//从视口中移除
	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
	if (ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}
	if (MultiplayerSessionSubsystem && MultiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

bool UReturnToMainMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	return true;
}

void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)//这是委托的回调函数，那边会返回执行成功没
{
	//执行不成功则代表没能成功退出游戏,则打开按钮
	if(!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if(World)
	{
		AGameModeBase * GameMode =  World->GetAuthGameMode<AGameModeBase>();
		if(GameMode)
		{
			GameMode->ReturnToMainMenuHost();//回到主菜单
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if(PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}


void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);//按下之后关掉这个退出按钮的选项

	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController * FirstPlayerController = World->GetFirstPlayerController();
		if(FirstPlayerController)
		{
			ABlasterCharacter * BlasterCharacter = Cast<ABlasterCharacter>(FirstPlayerController->GetPawn());
			if(BlasterCharacter)
			{
				BlasterCharacter->ServerLeaveGame();
				BlasterCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
			}
			else
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}

}


void UReturnToMainMenu::OnPlayerLeftGame()
{
	if (MultiplayerSessionSubsystem)
	{
		MultiplayerSessionSubsystem->DestroySession();//销毁会话
	}
}