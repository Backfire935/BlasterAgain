// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionSubsystem.generated.h"

//菜单到多人会话子系统之间的委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);

DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);

DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete,EOnJoinSessionCompleteResult::Type Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool , bWasSuccessful);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UMultiplayerSessionSubsystem();

	//
	//处理会话函数  菜单类会调用这些
	//	 
	//菜单访问子系统 调用创建会话函数 回调在线子系统设置并调用在线子系统接口函数来创建会话
	void CreateSession(int32 NumPublicConnections, FString MatchType);//能够连接的人数  匹配的游戏类型,就是房间名
	void FindSessions(int32 MaxSearchResults); //最大搜索结果
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);//
	void DestroySession();
	void StartSession();

	//
	//我们自己的为菜单类绑定回调的委托
	//
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete    MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

	int32 DesiredNumPublicConnections{};

	FString DesiredMatchType{};
protected:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName , bool bWasSuccessful);

private:
	
	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;

	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;


	//
	//添加在线会话接口委托列表 
	//绑定 多人玩家会话子系统 内部回调到这里
	//
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnFindSessionsCompleteDelegate  FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;

	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;


	bool bCreateSessionOnDestroy{ false };


	int32 LastNumPublicConnctions;

	FString LastMatchType;

	
	
};
