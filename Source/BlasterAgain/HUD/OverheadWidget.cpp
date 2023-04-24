// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole LocalRole = InPawn->GetLocalRole();
	FString Role;
	switch (LocalRole)
	{
	case ENetRole::ROLE_Authority :
		Role = FString("Authority");
		break;

		case ENetRole::ROLE_AutonomousProxy :
			Role = FString("AutonomousProxy");
			break;

		case ENetRole::ROLE_SimulatedProxy:
			Role = FString("SimulatedProxy");
			break;

		case ENetRole::ROLE_None:
			Role = FString("None");
			break;
	}

	FString LocalRoleString = FString::Printf(TEXT("Local Role: %s"), *Role);
	SetDisplayText(LocalRoleString);
}

void UOverheadWidget::ShowPlayerName(APawn *InPawn)
{

	APlayerState* PlayerState = InPawn->GetPlayerState();

	// ��ȡ�������
	if ( PlayerState != nullptr)
	{
		FString PlayerName = PlayerState->GetPlayerName();
		// ������Ը�����Ҫ�� FString ת��Ϊ std::string ����������
		// ...
		FString LocalRoleString = FString::Printf(TEXT("%s"), *PlayerName);
		SetDisplayText(LocalRoleString);
	}
	else
	{
		FString LocalRoleString = FString::Printf(TEXT("Unknown Player"));
		SetDisplayText(LocalRoleString);
	}

}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::NativeDestruct();
}
