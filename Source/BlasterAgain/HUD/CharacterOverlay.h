// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ElimText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountDownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingText;

	//蓝队分数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScore;

	//红队分数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamScore;

	//目标获胜分数
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TargetScore;

	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage; 

	UPROPERTY(meta = (BindWidgetAnim),Transient)
	UWidgetAnimation* HighPingAnimation;
};
