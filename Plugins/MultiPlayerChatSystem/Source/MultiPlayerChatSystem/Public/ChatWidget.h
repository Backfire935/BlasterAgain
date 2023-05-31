// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "ChatWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERCHATSYSTEM_API UChatWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
public:
	UPROPERTY(EditAnywhere,meta=(BindWidget))
	 class UPanelWidget*  Panel;

	UPROPERTY(EditAnywhere,meta=(BindWidget))
	 class UVerticalBox* VerticalBox;

	UPROPERTY(EditAnywhere,meta=(BindWidget))
	 class UScrollBox* ScrollBox;

	UPROPERTY(EditAnywhere,meta=(BindWidget))
	 class UHorizontalBox* HorizontalBox;

	UPROPERTY(EditAnywhere,meta=(BindWidget))
	class UTextBlock* TextBlock;//输入的内容

	UPROPERTY(EditAnywhere,meta=(BindWidget))
	class UEditableTextBox* EditableTextBox;


	UPROPERTY()
	class AStorageChatInfoPawn* InfoPawn;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AStorageChatInfoPawn> InfoPawnClass;

	friend class AStorageChatInfoPawn; 
	
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void InputWord();

	virtual bool OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent);
	

	//virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	virtual void NativeConstruct() override;
	//virtual bool Initialize() override;

	
private:
	bool bWrite = false;
	bool bCreate = false;
};
