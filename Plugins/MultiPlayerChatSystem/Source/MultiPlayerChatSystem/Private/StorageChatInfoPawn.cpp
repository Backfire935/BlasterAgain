// Fill out your copyright notice in the Description page of Project Settings.


#include "StorageChatInfoPawn.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"

AStorageChatInfoPawn::AStorageChatInfoPawn()
{
 	
	PrimaryActorTick.bCanEverTick = false;
	
}

void AStorageChatInfoPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AStorageChatInfoPawn, InputText);
}

void AStorageChatInfoPawn::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp,Warning,TEXT("ChatWidget:%d"),ChatWidget != nullptr);
}


void AStorageChatInfoPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AStorageChatInfoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AStorageChatInfoPawn::OnRep_InputWords()
{
	UE_LOG(LogTemp,Warning,TEXT("OnRep_InputWords"));
	if(ChatWidget)
	{
		UTextBlock* NewTextBlock = NewObject<UTextBlock>(this); // 创建新的文本块组件
		NewTextBlock->SetText(InputText); // 设置文本内容
		ChatWidget->ScrollBox->AddChild(NewTextBlock);
		UE_LOG(LogTemp,Warning,TEXT("ChatWidget"));

	}
	
}