// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

#include "Announcement.h"
#include "CharacterOverlay.h"
#include "ElimAnnouncement.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X /2.f, ViewportSize.Y / 2.f);
		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairsSpread;//准心偏移的浮点量
		FVector2D Spread;//准心偏移坐标
		if (HUDPackage.CrosshairsCenter)//开始画上下左右和中心的准心纹理
		{
			Spread = FVector2D(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsLeft)
		{
			Spread = FVector2D(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsRight)
		{
			Spread = FVector2D(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsTop)
		{
			Spread = FVector2D(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsBottom)
		{
			Spread = FVector2D(0.f, SpreadScaled); 
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

	}
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();//获取拥有的玩家控制器
	if (PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);//创建控件到角色上
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if(OwningPlayer && ElimAnnouncementClass)
	{
		//创建一个淘汰宣告组件
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnouncementClass);
		if(ElimAnnouncementWidget)
		{
			//调函数传参
			ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
			//添加到视口
			ElimAnnouncementWidget->AddToViewport();

			//将已有的信息向上移，为新消息让位置
			for(UElimAnnouncement* Msg : ElimMessages)
			{
				if(Msg && Msg->AnnouncementBox)
				{
					//获取画板槽
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if(CanvasSlot)
					{
					FVector2D Position = CanvasSlot->GetPosition();
						//原点为左上角，X向右Y向下
					FVector2D NewPosition(
						Position.X,
						Position.Y - CanvasSlot->GetSize().Y
						);
					CanvasSlot->SetPosition(NewPosition);
					}
				}
			}


			ElimMessages.Add(ElimAnnouncementWidget);

			//到时间的消息销毁
			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;
			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(
				ElimMsgTimer,
				ElimMsgDelegate,//触发的委托
				ElimAnnouncementTime,//时间
				false//定时器是否循环
			);
		}
	}
}


void ABlasterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
}

void ABlasterHUD::AddCharacterOverlay()//此函数由玩家控制器在经过开局倒计时后进入游戏状态后调用
{
	APlayerController * PlayerController = GetOwningPlayerController();//获取拥有的玩家控制器
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);//创建控件到角色上
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();//要设置的准心的纹理宽度等于从武器类那拿到的纹理的尺寸的宽度
	const float TextureHeight = Texture->GetSizeY();//同上
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - TextureWidth / 2.f +Spread.X,
		ViewportCenter.Y - TextureHeight / 2.f +Spread.Y
	);

	DrawTexture(
		Texture,//纹理
		TextureDrawPoint.X,//屏幕尺寸X
		TextureDrawPoint.Y,//屏幕尺寸Y
		TextureWidth,//纹理宽度
		TextureHeight,//纹理高度
		0.f,//纹理U
		0.f,//纹理V
		1.f,//纹理U宽度
		1.f,//纹理V高度
		CrosshairColor//画出来的纹理颜色
	);
}