// Fill out your copyright notice in the Description page of Project Settings.


#include "ChatWidget.h"


#include "Components/EditableTextBox.h"

#include "Components/VerticalBox.h"
#include "GameFramework/PlayerState.h"

#include "InputCoreTypes.h"
#include "StorageChatInfoPawn.h"




void UChatWidget::InputWord()
{
	bWrite = !bWrite;
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if(bWrite)//如果第一次按enter，正在输入
	{
		
		if(PlayerController)
		{  //FInputModeGameAndUI  FInputModeUIOnly
			PlayerController->SetInputMode(FInputModeGameAndUI());
			PlayerController->bShowMouseCursor = false;
			SetIsFocusable(true);
			EditableTextBox->SetFocus();
			// bIsFocusable = true;
		}
		//UE_LOG(LogTemp,Warning,TEXT("222"));
	}
	else//输入完了，第二次按回车
	{
		FText Text =  EditableTextBox->GetText();
		if(PlayerController && PlayerController->PlayerState)
		{
			SetIsFocusable(false);
			PlayerController->SetInputMode(FInputModeGameOnly());
			UE_LOG(LogTemp,Warning,TEXT("FInputModeGameOnly"));
			PlayerController->bShowMouseCursor = false;
			//bIsFocusable = true;
			EditableTextBox->SetText(FText());
			if(!Text.IsEmpty())
			{
				FString PlayerNameString = PlayerController->PlayerState->GetPlayerName();//获取玩家姓名
				PlayerNameString.Append(":");//添加:
				PlayerNameString.Append(Text.ToString());//添加文本内容
				FText PlayerNameText = FText::FromString(PlayerNameString);//转化回FText
				//设置文本内容
				if(InfoPawn)
				{
					InfoPawn->InputText = PlayerNameText;
					UE_LOG(LogTemp,Warning,TEXT("InfoPawn"));
				}
				
				UE_LOG(LogTemp,Warning,TEXT("%s"),*PlayerNameString);
			}
		}	
	}

	
}

void UChatWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 在这里实现自定义的UI逻辑
	if(!bCreate && InfoPawn)
	{
		InfoPawn->ChatWidget = this;
		UE_LOG(LogTemp, Warning, TEXT("InfoPawn->ChatWidget:%d"),InfoPawn->ChatWidget != nullptr);
		bCreate = true;
	}
	
}

bool UChatWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Enter)
	{
		// 在此处处理回车键逻辑
		UE_LOG(LogTemp, Warning, TEXT("Enter key pressed!"));
		InputWord();
		// 返回 true，表示已处理该按键事件
		return true;
	}

	// 返回 false，表示未处理该按键事件，继续传递给其他处理逻辑
	return false;
}




void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InfoPawn = GetWorld()->SpawnActor<AStorageChatInfoPawn>(InfoPawnClass);//去蓝图里把InfoPawnClass设置了
	
	UE_LOG(LogTemp, Warning, TEXT("InfoPawn:%d"),InfoPawn != nullptr);
}


