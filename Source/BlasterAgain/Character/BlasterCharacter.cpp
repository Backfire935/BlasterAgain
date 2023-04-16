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
	//������󵽽�ɫͷ��
	FollowCamera->SetupAttachment(GetMesh(), FName(TEXT("head")));

	//���������˳������
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
		Combat->Character = this;//�������ʼ������������趨�� CombatComponent�����Character������ֵ��˭.
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
	//����������� Set Player Name
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
		// �ڷ��������������� Set player name in server
		PlayerController->PlayerState->SetPlayerName(PlayerName);
		// ����ClientSetPlayerName�����ڿͻ�������������� use ClientSetPlayerName to set player name in client 
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
		// �ж��ĸ���ת��������ǰ��
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// ��ȡ��ǰ������
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, value);
	}

}

void ABlasterCharacter::MoveRight(const FInputActionValue& InputValue)
{
	float value = InputValue.GetMagnitude();

	if (Controller != nullptr && value != 0.f)
	{
		// �ж��ĸ���ת���������ҵ�
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// ��ȡ���ҵ�����
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
			Combat->EquipWeapon(OverlappingWeapon);//�������Server�Ͼ�ֱ��ִ��
		}
		else
		{
			ServerEquipButtonPressed();//��Ȼ��ִ��RPC  
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	//����ֻ�ڷ���˵��ã�����дHasAuthority 
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
			Combat->DropWeapon();//�������Server�Ͼ�ֱ��ִ��
		}
		else
		{
			ServerDropButtonPressed();//��Ȼ��ִ��RPC  
		}
	}
}

void ABlasterCharacter::ServerDropButtonPressed_Implementation()
{
	//����ֻ�ڷ���˵��ã�����дHasAuthority 
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
	//����Ƿ����ִ�У����൱�ڵ���һ�α��ؿͻ��ˣ�Ȼ��ֱ��return
	if(HasAuthority())
	{
		//����˵ı��ؾ��Ƿ�����Լ��Ŀͻ���
		ClientChangeView();
		return;
	}
	//����ǿͻ���ִ�У��÷����ȥ�ı䣬�ٴ��ݸ������ͻ��������ͬ����Ч��
		ServerChangeView();
}



void ABlasterCharacter::ServerChangeView_Implementation()
{
	//��Ҫ�޸ķ������ϵİ汾����Ϊ�������ϵİ汾������ͬ������Ŀͻ��˵ģ���Ŀͻ��˲�������ִ�ж����Ŀͻ���ͨ�ţ���Ŀͻ����õ��Ƿ���˵�����
		ChangeCameraView();
	//��ȥ������ִ�ж����Ŀͻ���ȥ����ʵ�ֹ���
	
		ClientChangeView();//ע�ʹ˺����󣬿ͻ��˽�ɫ��ͬ����ת���ǲ��ܸı��ӽǣ�ԭ����û���յ�Server������ClientChangeView()���Ըı䲻���ӽǣ����Ǻͽ�ɫ�ƶ���ص�������Ĭ��ʵ������ͬ���ģ��Ƿ���ת����������Server�˱��޸ĵ�ʱ��ͱ�ͬ�����ͻ����ˣ���˲���ҪServer������ClientChangeView()Ҳ�ı�����ת��
}

void ABlasterCharacter::ClientChangeView_Implementation()
{
	ChangeCameraView();
}

void ABlasterCharacter::ChangeCameraView()
{
	// �жϵ�ǰ���������������л�����һ�������
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
	OverlappingWeapon = Weapon;//����ǿͻ��ˣ�����ִ�������������Ϊ��������weapon���õ�����������ͻ��˴�����ײ����ڷ����ִ�У�Ȼ��OnRep���OverlappingWeapon���������ͻ���ȥ������Ϊ������ΪCOND_OwnerOnly������ֻ��Owner��Ӧ�Ŀͻ����õ���ֵ��OnRep�ɷ���˵��ã����ڷ����ִ��ֻ�ڿͻ���ִ�У�����Server���ᴥ��OnRep����
	//��ΪServer���ᴥ��OnRep��������������Server����Ҵ�������ײ��ͨ������ķ���������ʾ��
	//ֻ��Server�ϵı������ִ�У����Ϻ������������ڷ����AWeapon::OnSphereOverlap���õ�
	if(IsLocallyControlled())//Returns true if controlled by a local (not network) Controller.ʵ��ע���������û��Ӱ�죬��Ϊ������������Ķ����������ȷ���˴������е����ʱ��ֻ�����Ƿ���������У��ͻ���ѹ������������������������ս̳̱������������������Ϊ�������������ģ������Ķ���server
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
	//ʹ�ÿ�����������תpitch��yaw
	//bUseControllerRotationPitch = true;
	//bUseControllerRotationYaw = true;

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{

			//�����ӳ������ظ����
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(IMC_Action, 0);
			Subsystem->AddMappingContext(IMC_MoveBase, 0);
		}
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent)
	{

		//�ƶ�
		EnhancedInputComponent->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveForward);
		EnhancedInputComponent->BindAction(IA_MoveRight, ETriggerEvent::Triggered, this, &ABlasterCharacter::MoveRight);
		//��ת
		EnhancedInputComponent->BindAction(IA_LookRightRate, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookRightRate);
		EnhancedInputComponent->BindAction(IA_LookUpRate, ETriggerEvent::Triggered, this, &ABlasterCharacter::LookUpRate);
		//��Ծ
		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Started, this, &ABlasterCharacter::OnJump);

		EnhancedInputComponent->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ABlasterCharacter::OnJumpStoping);

		//���
		EnhancedInputComponent->BindAction(IA_FirePressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::FirePressed);

		EnhancedInputComponent->BindAction(IA_FireReleased, ETriggerEvent::Triggered, this, &ABlasterCharacter::FireReleased);

		//�Ҽ����ؾ�
		EnhancedInputComponent->BindAction(IA_AimingPressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputAimingPressed);

		EnhancedInputComponent->BindAction(IA_AimingReleased, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputAimingReleased);
		//shift����	
		EnhancedInputComponent->BindAction(IA_ShiftPressed, ETriggerEvent::Triggered, this, &ABlasterCharacter::LowSpeedWalk);

		EnhancedInputComponent->BindAction(IA_ShiftReleased, ETriggerEvent::Triggered, this, &ABlasterCharacter::NormalSpeedWalk);

		//��G��������
		EnhancedInputComponent->BindAction(IA_DropWeapon, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputDropWeapon);

		//��R��������
		EnhancedInputComponent->BindAction(IA_Reload, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputReload);

		//��V�л��ӽ�
		EnhancedInputComponent->BindAction(IA_ChangeView, ETriggerEvent::Triggered, this, &ABlasterCharacter::InputShiftView);

		//��Fʰȡ����
		EnhancedInputComponent->BindAction(IA_Equip, ETriggerEvent::Triggered, this, &ABlasterCharacter::EquipButtonPressed);


		// �����ͨ������"ETriggerEvent"ö��ֵ���󶨵��˴������ⴥ�����¼�
		//Input->BindAction(AimingInputAction, ETriggerEvent::Triggered, this, &AFPSBaseCharacter::SomeCallbackFunc);

	}
}



