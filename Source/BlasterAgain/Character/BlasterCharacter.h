// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"

#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTERAGAIN_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//设置同步
	virtual void PostInitializeComponents() override;


#pragma region InputFunction
protected:
	virtual void BeginPlay() override;

	///输入绑定
	void MoveForward(const FInputActionValue& InputValue);

	void MoveRight(const FInputActionValue& InputValue);

	void LookRightRate(const FInputActionValue& InputValue);

	void LookUpRate(const FInputActionValue& InputValue);

	void OnJump(const FInputActionValue& InputValue);

	void OnJumpStoping(const FInputActionValue& InputValue);

	//按下左shift静步慢走
	void RunSpeedWalk(const FInputActionValue& InputValue);

	//松开左shift正常行走
	void NormalSpeedWalk(const FInputActionValue& InputValue);
	///

	void FirePressed(const FInputActionValue& InputValue);

	void FireReleased(const FInputActionValue& InputValue);

	void InputAimingPressed(const FInputActionValue& InputValue);

	void InputAimingReleased(const FInputActionValue& InputValue);

	void CrouchButtonPressed(const FInputActionValue& InputValue);
	//按G丢弃武器
	void InputDropWeapon();

	//按R武器换弹
	void InputReload(const FInputActionValue& InputValue);

	//按V切换视角
	void InputShiftView(const FInputActionValue& InputValue);

	//按V切换视角
	void EquipButtonPressed(const FInputActionValue& InputValue);
	///

#pragma endregion InputFunction

public:

#pragma region RPC
	UFUNCTION(Client, Reliable)
		void ClientSetName(const FString& Name);

	UFUNCTION(Server, Reliable)
		void	ServerSetPlayerName(const FString& PlayerName);

	//RPC角色减速
	UFUNCTION(Server, Reliable)
		void ServerRunSpeed();

	//RPC角色恢复速度
	UFUNCTION(Server, Reliable)
		void ServerNormalSpeedWalk();

	//切换第三人称后客户端旋转朝向运动
	UFUNCTION(Server, Reliable)
		void ServerChangeView();

	UFUNCTION(Client, Reliable)
		void ClientChangeView();

	UFUNCTION()
	void ChangeCameraView();

	UFUNCTION(Server, Reliable)
		void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
		void ServerDropButtonPressed();



	//初始设置角色材质
	UFUNCTION(Server, Reliable)
		void ServerSetMaterial();

	UFUNCTION(Client, Reliable)
		void ClientSetMaterial();

#pragma endregion RPC

protected:
	//游戏内鼠标灵敏度
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Input")
		float TurnRateGamepad = 5.f;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float RunSpeed = 437.5f;//奔跑速度

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float RunAimSpeed = 337.5f;//奔跑瞄准速度

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float WalkSpeed = 210.f;//行走速度

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float WalkAimSpeed = 180.f;//行走瞄准速度

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float CrouchSpeed = 180.f;//下蹲速度

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float CrouchAimSpeed = 150.f;//下蹲瞄准速度

	UPROPERTY(Replicated)
	bool IsRunning = false;

	void AimOffset(float DeltaTime);
	float AO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

private:

	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class  USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
		class UCameraComponent* TPSCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		 class UWidgetComponent* OverHeadWidget;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Player Name",meta = (AllowPrivateAccess = "true"))
		FString LocalPlayerName = TEXT("Unknown Player");

	UPROPERTY(ReplicatedUsing= OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
		void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
		class UCombatComponent* CombatComp;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "EffectMaterials",meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* TransparentMaterial;//透明材质

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "EffectMaterials",meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* NormalMaterialUp;//上半身正常材质

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "EffectMaterials",meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* NormalMaterialDown;//下半身正常材质
	
#pragma region EnhancedInput
	///增强输入
	//两个映射表
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Context", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputMappingContext> IMC_Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Context", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputMappingContext> IMC_MoveBase;

	//上下移动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_MoveForward;

	//左右移动
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_MoveRight;

	//上下视角
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_LookUpRate;

	//左右视角
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction>  IA_LookRightRate;

	//跳跃
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Jump;


	//开火
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_FirePressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_FireReleased;

	//开镜
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_AimingPressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_AimingReleased;

	//按下shift静步减速
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ShiftPressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ShiftReleased;

	//按下Ctrl下蹲
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_CrouchPressed;

	//按下G丢弃武器
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_DropWeapon;

	//按下R武器换弹
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Reload;

	//按下V切换视角
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ChangeView;

	//按下F拾取武器
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Equip;
	///
#pragma endregion EnhancedInput



public:
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();

	bool IsAiming();
	
	bool IsFirstPerson();//获取是否是第一人称

	AWeapon* GetEquippedWeapon();
	
	FORCEINLINE bool GetIsCrouch() { return bIsCrouched; }//获取是否是下蹲的
	FORCEINLINE float GetWalkSpeed() { return WalkSpeed; }//行走速度
	FORCEINLINE float GetRunSpeed() { return RunSpeed; }//奔跑速度
	FORCEINLINE float GetCrouchSpeed() { return CrouchSpeed; }//下蹲速度
	FORCEINLINE float GetRunAimSpeed() { return RunAimSpeed; }//奔跑瞄准速度
	FORCEINLINE float GetCrouchAimSpeed() { return CrouchAimSpeed; }//下蹲瞄准速度
	FORCEINLINE float GetWalkAimSpeed() { return WalkAimSpeed; }//行走瞄准速度
	FORCEINLINE float GetIsRunning() { return IsRunning; }//是否奔跑
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch;}

};


