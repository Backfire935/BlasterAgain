// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "BlasterAgain/BlasterTypes/CombatState.h"
#include "BlasterAgain/BlasterTypes/Team.h"
#include "BlasterAgain/BlasterTypes/TurningInPlace.h"
#include "Components/TimelineComponent.h"

#include "BlasterCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

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

	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
	virtual void OnRep_ReplicatedMovement() override;//将角色转向应用到模拟代理的回调

	void Elim(bool bPlayerLeftGame);//这个只在server上调用
	void DropOrDestroyWeapon(class AWeapon* Weapon);
	void DropOrDestroyWeapons();
	//销毁武器的问题

	void SetSpawnPoint();//设置出生点
	void OnPlayerStateInitialized();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);//玩家淘汰时

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;//用于在游戏将结束时禁用,移动，开火，瞄准，换弹，装备武器，跳跃，下蹲

	UFUNCTION(BlueprintImplementableEvent)//可在蓝图或关卡蓝图中实现的函数
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();//更新生命值
	void UpdateHUDShield();//更新护盾

	void UpdateHUDAmmo();//更新弹药HUD

	void SpawnDefaultWeapon();
	void PlayHitReactMontage();
	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();//客户端RPC请求服务端离开游戏

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team);

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
	//获取Yaw的offset
	void SimProxiesTurn();
	float AO_Yaw;
	float AO_Pitch;
	float	InterpAO_Yaw;
	FRotator StartingAimRotation;
	void TurnInPlace(float DeltaTime);//转向函数
	 ETurningInPlace TurningInPlace;//转向的枚举类型

	UFUNCTION()
	void ReceiveDamage(AActor *DamagedActor, float Damage, const UDamageType * DamageType, class AController* InstigatorController, AActor * DamageCauser);

	void PollInit();//初始化相关类
	


	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	


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

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensationComp;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "EffectMaterials",meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* TransparentMaterial;//透明材质

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "EffectMaterials",meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* NormalMaterialUp;//上半身正常材质

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "EffectMaterials",meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* NormalMaterialDown;//下半身正常材质

		//几种蒙太奇动画
		UPROPERTY(EditAnywhere, category = Combat )
		class UAnimMontage* FireWeaponMontage;

		UPROPERTY(EditAnywhere, category = Combat)
			 UAnimMontage* HitReactMontage;

		UPROPERTY(EditAnywhere, category = Combat)
			 UAnimMontage* ElimMontage;

		UPROPERTY(EditAnywhere, category = Combat)
			UAnimMontage* ReloadMontage;

		UPROPERTY(EditAnywhere, category = Combat)
			UAnimMontage* ThrowGrenadeMontage;

		void HideCameraIfCharacterClose();

		UPROPERTY(EditAnywhere, category = Combat)
		float CameraThreshold = 200.f;//相机阈值

		bool bRotateRootBone;

		float TurnThreshold = 3.5f;//达到转向要求的差值角度

		FRotator ProxyRotationLastFrame;//上一帧的代理旋转
		FRotator ProxyRotation;

		float ProxyYaw;

		float TimeSinceLastMovementReplication;

		float CalculateSpeed();

	//生命值
		UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f;

		UPROPERTY(ReplicatedUsing = OnRep_Health,  EditAnywhere, Category = "Player Stats")
		float Health = 100.f;

		UFUNCTION()
		void OnRep_Health(float LastHealth);

	//护盾
		UPROPERTY(EditAnywhere, Category = "Player Stats")
			float MaxShield = 100.f;

		UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
			float Shield = 100.f;


		UFUNCTION()
			void OnRep_Shield(float LastShield);


		class	 ABlasterPlayerController* BlasterPlayerController;

		bool bElimed = false;

		FTimerHandle ElimTimer;

		UPROPERTY(EditDefaultsOnly)
		float ElimDelay = 3.f;

		void ElimTimerFinished();

		bool bLeftGame = false;

		UPROPERTY(VisibleAnywhere)
		UTimelineComponent* DissolveTimeline;

		UPROPERTY(EditAnywhere)
			UCurveFloat* DissolveCurve;

		//溶解效果
		FOnTimelineFloat DissolveTrack;//溶解效果的时间轴

		UFUNCTION()
		void UpdateDissolveMaterial(float DissolveValue);
		void StartDissolve();

		//在运行时可以改变的动态材质实例
		UPROPERTY(VisibleAnywhere, category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;//动态溶解材质实例

		//在蓝图中设置的动态材质实例
		UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstance;

		//队伍颜色
	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* RedDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* OriginalMaterial;

		//淘汰效果
		UPROPERTY(EditAnywhere)
		UParticleSystem* ElimBotEffect;

		UPROPERTY(VisibleAnywhere)
		UParticleSystemComponent* ElimBotComponent;

		UPROPERTY(EditAnywhere)
		class USoundCue* ElimBotSound;

		class ABlasterPlayerState *BlasterPlayerState;

		UPROPERTY(EditAnywhere)
		class UNiagaraSystem* CrownSystem;

		UPROPERTY()
		class UNiagaraComponent* CrownComponent;

	//投掷物
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* AttachedGrenade;

	UPROPERTY(EditAnywhere,Category= "Combat")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	//是否开启友军伤害
	UPROPERTY(EditAnywhere, Category = "Team")
	bool bTeamDamage =false;

	//友军伤害倍率
	UPROPERTY(EditAnywhere, Category = "Team")
		float TeamDamageRate = 1.f;
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

	FVector GetHitTarget() const;

	ECombatState GetCombatState() const;

	bool IsHoldingTheFlag();
	bool IsLocallyReloading();
	ETeam GetTeam();
	void SetHoldingTheFlag(bool bHolding);
	
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
	
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimed() const { return bElimed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE void SetHealth(float Amount)  { Health = Amount; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	FORCEINLINE bool GetDisableGamePlay() const { return bDisableGameplay; }

	FORCEINLINE UCombatComponent* GetCombat() const { return CombatComp; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComp; }
	
};


