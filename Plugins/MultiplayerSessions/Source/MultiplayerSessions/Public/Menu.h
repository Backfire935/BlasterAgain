// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 100, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPerson/Maps/LobbyMap")) );
protected:
	
	virtual bool Initialize() override;	//初始化按钮控件

	virtual void NativeDestruct() override ; //打开新关卡时销毁所有当前关卡
	//UE5.1 删除OnLevelRemoveFromWorld函数，使用NativeDestruct函数替换，5.1版本以下换回OnLevelRemoveFromWorld，感谢epic讨论区老哥救我一命
	// https://forums.unrealengine.com/t/where-is-uuserwidget-onlevelremovedfromworld-in-5-1/692215/7
	 //virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override ;  //这个地方是大坑 Removed少写一个d 改bug改两小时 因为真有不要d的函数

	//
	//为多人会话子系统委托的回调函数
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	//绑定蓝图中的按钮到下面的变量中  同名绑定
	UPROPERTY(meta = (BindWidget))	
	class UButton* HostButton;	
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
	UFUNCTION()
		void HostButtonClicked();
	UFUNCTION()
		void JoinButtonClicked();
	UFUNCTION()
		void ExitButtonClicked();

	void MenuTearDown();

	//用于处理所有的在线会话功能的子系统
	class UMultiplayerSessionSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections{ 4 };

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{ TEXT("FreeForAll") };
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString PathToLobby{ TEXT("")};
};
	