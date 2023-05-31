// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChatWidget.h"
#include "GameFramework/Pawn.h"
#include "StorageChatInfoPawn.generated.h"

UCLASS()
class MULTIPLAYERCHATSYSTEM_API AStorageChatInfoPawn : public APawn
{
	GENERATED_BODY()

public:

	UPROPERTY(ReplicatedUsing = OnRep_InputWords)
	 FText InputText;

	UFUNCTION()
	void OnRep_InputWords();//�����ڸ����ͻ���������µ���Ϣ

	UPROPERTY()
	class UChatWidget * ChatWidget;
protected:

	virtual void BeginPlay() override;

public:	
	AStorageChatInfoPawn();
	
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//����ͬ��
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
