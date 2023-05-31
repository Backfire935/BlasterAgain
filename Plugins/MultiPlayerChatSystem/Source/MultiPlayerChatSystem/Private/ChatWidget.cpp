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
	if(bWrite)//�����һ�ΰ�enter����������
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
	else//�������ˣ��ڶ��ΰ��س�
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
				FString PlayerNameString = PlayerController->PlayerState->GetPlayerName();//��ȡ�������
				PlayerNameString.Append(":");//���:
				PlayerNameString.Append(Text.ToString());//����ı�����
				FText PlayerNameText = FText::FromString(PlayerNameString);//ת����FText
				//�����ı�����
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

	// ������ʵ���Զ����UI�߼�
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
		// �ڴ˴�����س����߼�
		UE_LOG(LogTemp, Warning, TEXT("Enter key pressed!"));
		InputWord();
		// ���� true����ʾ�Ѵ���ð����¼�
		return true;
	}

	// ���� false����ʾδ����ð����¼����������ݸ����������߼�
	return false;
}




void UChatWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InfoPawn = GetWorld()->SpawnActor<AStorageChatInfoPawn>(InfoPawnClass);//ȥ��ͼ���InfoPawnClass������
	
	UE_LOG(LogTemp, Warning, TEXT("InfoPawn:%d"),InfoPawn != nullptr);
}


