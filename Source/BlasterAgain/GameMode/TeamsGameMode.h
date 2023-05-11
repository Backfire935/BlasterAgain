// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()

public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage, bool bTeamDamage , float TeamDamageRate) override;
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, class ABlasterPlayerController* AttackerController) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//使用属性网络复制，需要使用此函数进行注册


	//获胜分数
	UPROPERTY(ReplicatedUsing= OnRep_TargetScore,EditAnywhere)
	float TargetScore = 100.f;

	UFUNCTION()
	void OnRep_TargetScore();

	//获取获胜的分数
	FORCEINLINE float GetTargetScore() const { return TargetScore; }



protected:
	virtual void HandleMatchHasStarted() override;

};
