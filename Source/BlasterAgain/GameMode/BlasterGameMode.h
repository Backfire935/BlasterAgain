// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
namespace MatchState
{
	extern  BLASTERAGAIN_API const FName Cooldown;//已经到达了匹配的持续时间，播放赢家并开始结束游戏计时

}
UCLASS()
class BLASTERAGAIN_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ABlasterGameMode();

	virtual void Tick(float Deltatime) override;

	virtual void PlayerEliminated(class ABlasterCharacter * ElimmedCharacter, class ABlasterPlayerController * VictimController, class ABlasterPlayerController* AttackerController);

	virtual void RequestRespawn(class ACharacter * ElimmedCharacter, AController *ElimmedController);

	void PlayerLeftGame(class ABlasterPlayerState * PlayerLeaving);//离开游戏

	virtual float CalculateDamage(AController * Attacker, AController * Victim, float BaseDamage,bool bTeamDamage,float TeamDamageRate);//是否开启友伤,友伤只有0.6倍

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;//只能从默认蓝图中设置,热身阶段的时间

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;//只能从默认蓝图中设置,热身阶段的时间

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;//只能从默认蓝图中设置,热身阶段的时间

	float LevelStartingTime = 0.f;

	//是否开启团队模式
	UPROPERTY(EditAnywhere)
	bool bTeamsMatch = false;

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;//倒计时的时间

	public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
