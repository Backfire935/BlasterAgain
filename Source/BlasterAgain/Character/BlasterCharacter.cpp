#pragma once

#include "BlasterCharacter.h"
#include "EnhancedInputSubsystems.h"
#include"EnhancedInputComponent.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "GameFramework/PlayerState.h"
#include"Components/WidgetComponent.h"
#include "BlasterAgain/Weapon/Weapon.h"
#include "BlasterAgain/BlasterComponent/CombatComponent.h"

#pragma region Init
ABlasterCharacter::ABlasterCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	//摄像机绑到角色头上
	FollowCamera->SetupAttachment(GetMesh(), FName(TEXT("head")));

	//创建第三人称摄像机
	TPSCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TPSCamera"));
	TPSCamera->SetupAttachment(CameraBoom);

	FollowCamera->bUsePawnControlRotation = true;


	//bUseControllerRotationYaw = false;
	//GetCharacterMovement()->bOrientRotationToMovement = true;

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverHeadWidget->SetupAttachment(GetMesh());

	FollowCamera->SetActive(true);
	FollowCamera->Activate();
	TPSCamera->SetActive(false);
	TPSCamera->Deactivate();

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly); 
	
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character = this;//在请求初始化组件函数中设定了 CombatComponent组件中Character变量的值是谁.
	}

}



void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	ServerSetPlayerName(LocalPlayerName);

}


void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

#pragma endregion Init

#pragma region PlayerName
void ABlasterCharacter::ClientSetName_Implementation(const FString& Name)
{
	//设置玩家名称 Set Player Name
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController != nullptr)
	{
		PlayerController->PlayerState->SetPlayerName(Name);
	}
}

void ABlasterCharacter::ServerSetPlayerName_Implementation(const FString& PlayerName)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		// 在服务端设置玩家名称 Set player name in server
		PlayerController->PlayerState->SetPlayerName(PlayerName);
		// 调用ClientSetPlayerName函数在客户端设置玩家名称 use ClientSetPlayerName to set player name in client 
		ClientSetName(PlayerName);
	}
}
#pragma endregion PlayerName

#pragma region Input

void ABlasterCharacter::MoveForward(const FInputActionValue& InputValue)
{
	float value = InputValue.GetMagnitude();

	if ((Controller != nullptr) && (value != 0.f))
	{
		// 判断哪个旋转方向是向前的
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// 获取向前的向量
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
	}

}

void ABlasterCharacter::MoveRight(const FInputActionValue& InputValue)
{
	float value = InputValue.GetMagnitude();

	if (Controller != nullptr && value != 0.f)
	{
		// 判断哪个旋转方向是向右的
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// 获取向右的向量
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, value);
	}

}

void ABlasterCharacter::LookRightRate(const FInputActionValue& InputValue)
{
	AddControllerYawInput(InputValue.GetMagnitude() * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABlasterCharacter::LookUpRate(const FInputActionValue& InputValue)
{
	AddControllerPitchInput(InputValue.GetMagnitude() * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABlasterCharacter::OnJump(const FInputActionValue& InputValue)
{
	Jump();
}

void ABlasterCharacter::OnJumpStoping(const FInputActionValue& InputValue)
{
	StopJumping();
}



void ABlasterCharacter::LowSpeedWalk(const FInputActionValue& InputValue)
{

	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	ServerLowSpeedWalk();
}

void ABlasterCharacter::NormalSpeedWalk(const FInputActionValue& InputValue)
{

	GetCharacterMovement()->MaxWalkSpeed = 450.f;
	ServerNormalSpeedWalk();
}


void ABlasterCharacter::ServerLowSpeedWalk_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}


void ABlasterCharacter::ServerNormalSpeedWalk_Implementation()
{
	GetCharacterMovement()->MaxWalkSpeed = 450.f;
}

void ABlasterCharacter::EquipButtonPressed(const FInputActionValue& InputValue)
{
	if(Combat)
	{
		if(HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);//如果就在Server上就直接执行
		}
		else
		{
			ServerEquipButtonPressed();//不然就执行RPC  
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	//反正只在服务端调用，不用写HasAuthority 
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::InputDropWeapon()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->DropWeapon();//如果就在Server上就直接执行
		}
		else
		{
			ServerDropButtonPressed();//不然就执行RPC  
		}
	}
}

void ABlasterCharacter::ServerDropButtonPressed_Implementation()
{
	//反正只在服务端调用，不用写HasAuthority 
	if (Combat)
	{
		Combat->DropWeapon();
	}
}


#pragma endregion Input

#pragma  region notstart
void ABlasterCharacter::FirePressed(const FInputActionValue& InputValue)
{

}

void ABlasterCharacter::FireReleased(const FInputActionValue& InputValue)
{

}

void ABlasterCharacter::InputAimingPressed(const FInputActionValue& InputValue)
{

}

void ABlasterCharacter::InputAimingReleased(const FInputActionValue& InputValue)
{

}



void ABlasterCharacter::InputReload(const FInputActionValue& InputValue)
{

}
#pragma  endregion notstart

#pragma region ChangeView
void ABlasterCharacter::InputShiftView(const FInputActionValue& InputValue)
{
	//如果是服务端执行，就相当于调用一次本地客户端，然后直接return
	if(HasAuthority())
	{
		//服务端的本地就是服务端自己的客户端
		ClientChangeView();
		return;
	}
	//如果是客户端执行，让服务端去改变，再传递给各个客户端来完成同步的效果
		ServerChangeView();
}



void ABlasterCharacter::ServerChangeView_Implementation()
{
	//需要修改服务器上的版本，因为服务器上的版本是用来同步给别的客户端的，别的客户端不与正在执行动作的客户端通信，别的客户端拿的是服务端的数据
		ChangeCameraView();
	//再去让正在执行动作的客户端去真正实现功能
	
		ClientChangeView();//注释此函数后，客户端角色能同步旋转但是不能改变视角，原因是没有收到Server发来的ClientChangeView()所以改变不了视角，但是和角色移动相关的属性是默认实现网络同步的，是否旋转属性在上面Server端被修改的时候就被同步到客户端了，因此不需要Server发来的ClientChangeView()也改变了旋转。
}

void ABlasterCharacter::ClientChangeView_Implementation()
{
	ChangeCameraView();
}

void ABlasterCharacter::ChangeCameraView()
{
	// 判断当前激活的摄像机，并切换到另一个摄像机
	if (FollowCamera->IsActive())
	{
		FollowCamera->SetActive(false);
		FollowCamera->Deactivate();
		TPSCamera->SetActive(true);
		TPSCamera->Activate();
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
	}
	else
	{
		TPSCamera->SetActive(false);
		TPSCamera->Deactivate();
		FollowCamera->SetActive(true);
		FollowCamera->Activate();
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;

	}
}




#pragma endregion ChangeView


#pragma region ShowOrHidePickupWidget

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	OverlappingWeapon = Weapon;//如果是客户端，不会执行这个函数，因为是无主的weapon调用的这个函数，客户端触发碰撞后会在服务端执行，然后OnRep会把OverlappingWeapon传到其他客户端去，又因为被定义为COND_OwnerOnly，所以只有Owner对应的客户端拿到了值，OnRep由服务端调用，不在服务端执行只在客户端执行，所以Server不会触发OnRep函数
	//因为Server不会触发OnRep函数，所以若是Server的玩家触发的碰撞，通过下面的方法进行显示。
	//只有Server上的本地玩家执行，加上函数本身是由在服务端AWeapon::OnSphereOverlap调用的
	if(IsLocallyControlled())//Returns true if controlled by a local (not network) Controller.实测注释这个函数没有影响，因为调用这个函数的对象的特殊性确保了代码运行到这的时候只可能是服务端在运行，客户端压根不会调用这个函数，这里参照教程保留这个函数。特殊性为，武器是无主的，无主的都归server
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
		
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

#pragma endregion ShowOrHidePickupWidget

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//使用控制器控制旋转pitch和yaw
	//bUseControllerRotationPitch = true;
	//bUseControllerRotationYaw = true;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{

			//先清除映射避免重复添加
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(IMC_Action, 0);
			Subsystem->AddMappingContext(IMC_MoveBase, 0);
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent)
	{

		//移动
		EnhancedInputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveForward);
		EnhancedInputComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveRight);
		//旋转
		EnhancedInputComponent->BindAction(IA_LookRightRate, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookRightRate);
		EnhancedInputComponent->BindAction(IA_LookUpRate, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookUpRate);
		//跳跃
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ABlasterCharacter::OnJump);

		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ABlasterCharacter::OnJumpStoping);

		//射击
		EnhancedInputComponent->BindAction(IA_FirePressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::FirePressed);

		EnhancedInputComponent->BindAction(IA_FireReleased, ETriggerEvent::Triggered, this, &ABlasterCharacter::FireReleased);

		//右键开关镜
		EnhancedInputComponent->BindAction(IA_AimingPressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputAimingPressed);

		EnhancedInputComponent->BindAction(IA_AimingReleased, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputAimingReleased);
		//shift静步	
		EnhancedInputComponent->BindAction(IA_ShiftPressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::LowSpeedWalk);

		EnhancedInputComponent->BindAction(IA_ShiftReleased, ETriggerEvent::Triggered, this, &ABlasterCharacter::NormalSpeedWalk);

		//按G丢弃武器
		EnhancedInputComponent->BindAction(IA_DropWeapon, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputDropWeapon);

		//按R武器换弹
		EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputReload);

		//按V切换视角
		EnhancedInputComponent->BindAction(IA_ChangeView, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputShiftView);

		//按F拾取武器
		EnhancedInputComponent->BindAction(IA_Equip, ETriggerEvent::Triggered, this, &ABlasterCharacter::EquipButtonPressed);


		// 你可以通过更改"ETriggerEvent"枚举值，绑定到此处的任意触发器事件
		//Input->BindAction(AimingInputAction, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::SomeCallbackFunc);

	}
}



