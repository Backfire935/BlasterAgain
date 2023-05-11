// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterAgain/BlasterTypes/Team.h"
#include "GameFramework/PlayerStart.h"
#include "TeamPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	ETeam Team;
};
