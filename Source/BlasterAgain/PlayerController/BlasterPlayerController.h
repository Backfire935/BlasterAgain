// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);
/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//设置同步

	void PlayerCameraShake(TSubclassOf<UCameraShakeBase> CameraShake);

	void SetHUDHealth(float Health, float MaxHealth);//设置生命值

	void SetHUDShield(float Shield, float MaxShield);//设置生命值

	void SetHUDScore(float Score);//设置击杀数,这里用float是因为SetScore()默认用的float

	void SetHUDDefeats(int32 Defeats);//设置死亡次数

	void SetHUDWeaponAmmo(int32 Ammo);//设置当前弹夹弹药

	void SetHUDCarriedAmmo(int32 Ammo);//设置当前弹夹弹药

	void SetHUDMatchCountDown(float CountdownTime);//设置对局内开始时间

	void SetHUDAnnouncementCountdown(float CountdownTime);//设置游戏结束重置时间

	void SetHUDGrenades(int32 Grenades);//设置手雷的数量

	virtual void OnPossess(APawn* InPawn) override; 

	void SetElimText(bool bElim);//设置淘汰时的HUD显示
	
	virtual  void Tick(float DeltaTime) override;

	virtual void ReceivedPlayer() override;//在接收玩家的时候就执行GetServerTime，目的是尽快与服务端时间保持一致

	virtual float GetServerTime();//与服务器时间同步
	/*
	*服务端和客户端之间的异步时间计算
	*/
	UFUNCTION(Server,Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);//传入的是客户端发送请求时的时间，要求获取服务端的时间

	UFUNCTION(Client, Reliable)
		void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);//将服务端当前的时间发送给客户端，来回应客户端发起的ServerRequestServerTime请求

	float ClientServerDelta = 0.f;//客户端和服务端的时间差异

	UPROPERTY(EditAnywhere,Category = Time)
	float TimeSyncFrequency = 5.f;//每隔一段时间自动检查一次与服务器的时差，默认是5s

	float TimeSyncRunningTime = 0.f;//距离上一次检查的时间

	void CheckTimeSync(float DeltaTime);//检查时间同步的函数

	void OnMatchStateSet(FName State, bool bTeamMatch = false);
	void HandleMatchHasStarted(bool bTeamMatch = false);

	UFUNCTION(Server,Reliable)
	void ServerCheckMatchState();//服务端检查游戏状态

		UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float Cooldown,float StartingTime);//客户端中途加入游戏时同步数据

		void HandleCooldown();

		void HighPingWarning(float ping);
		void StopHighPingWarning();
		void CheckPing(float DeltaTime);

		float SingleTripTime = 0.f;

		FHighPingDelegate HighPingDelegate;

		void BroadcastElim(APlayerState* Attacker, APlayerState* Victim); 

	//关于HUD中队伍分数的显示
		void HideTeamScores();
		void InitTeamScores();
		void SetHUDRedTeamScores(int32 RedScore);
		void SetHUDBlueTeamScores(int32 BlueScore);
		void SetHUDTargetScore(int32 Score);

	protected:
	virtual void BeginPlay() override;

	void SetHUDTime();

	void PollInit();//用于检查CharacterOverlay用户控件是否创建，若创建好了再应用其中的数据值

	virtual void SetupInputComponent() override;

	void ShowReturnToMainMenu();

	UFUNCTION(Client,Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	UPROPERTY(ReplicatedUsing=OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<class ABlasterPlayerState*>& Players);
	FString GetTeamsInfoText(class ABlasterGameState * BlasterGameState);

	void SetInputWord();
	
private:
	/// <summary>
	/// 返回主菜单
	/// </summary>
	UPROPERTY(EditAnywhere,Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMenuOpen = false;

	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
		class ABlasterGameMode* BlasterGameMode;

	UPROPERTY()
		class ATeamsGameMode* TeamGameMode;

	float LevelStartingTime;//游戏开始的时间

	float MatchTime = 0.f;//

	float WarmupTime = 0.f;//热身时间

	uint32 CountdownInt = 0;

	float CooldownTime = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;//用于追踪游戏的状态

	UFUNCTION()
	void OnRep_MatchState();//让客户端也知道游戏的状态

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;


	float HUDHealth;
	float HUDMaxHealth;
	bool bInitializeHealth = false;

	float HUDShield;
	float HUDMaxShield;
	bool bInitializeShield = false;

	float HUDScore;
	bool bInitializeScore = false;

	int32 HUDDefeats;
	bool bInitializeDefeats = false;

	int32 HUDGrenades;
	bool bInitializeGrenades = false;

	float HUDCarriedAmmo;
	bool bInitializeCarriedAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;

	float HighPingRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	float PingAnimationRunningTime = 0.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 10.f;//间隔十秒检查一次ping值

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 100.f; //高ping警告的阈值,100以下都能玩
};
