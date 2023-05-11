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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//����ͬ��
	
	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	void PlayThrowGrenadeMontage();
	virtual void OnRep_ReplicatedMovement() override;//����ɫת��Ӧ�õ�ģ�����Ļص�

	void Elim(bool bPlayerLeftGame);//���ֻ��server�ϵ���
	void DropOrDestroyWeapon(class AWeapon* Weapon);
	void DropOrDestroyWeapons();
	//��������������

	void SetSpawnPoint();//���ó�����
	void OnPlayerStateInitialized();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);//�����̭ʱ

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;//��������Ϸ������ʱ����,�ƶ���������׼��������װ����������Ծ���¶�

	UFUNCTION(BlueprintImplementableEvent)//������ͼ��ؿ���ͼ��ʵ�ֵĺ���
	void ShowSniperScopeWidget(bool bShowScope);

	void UpdateHUDHealth();//��������ֵ
	void UpdateHUDShield();//���»���

	void UpdateHUDAmmo();//���µ�ҩHUD

	void SpawnDefaultWeapon();
	void PlayHitReactMontage();
	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();//�ͻ���RPC���������뿪��Ϸ

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast,Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team);

#pragma region InputFunction
protected:
	virtual void BeginPlay() override;

	///�����
	void MoveForward(const FInputActionValue& InputValue);

	void MoveRight(const FInputActionValue& InputValue);

	void LookRightRate(const FInputActionValue& InputValue);

	void LookUpRate(const FInputActionValue& InputValue);

	void OnJump(const FInputActionValue& InputValue);

	void OnJumpStoping(const FInputActionValue& InputValue);

	//������shift��������
	void RunSpeedWalk(const FInputActionValue& InputValue);

	//�ɿ���shift��������
	void NormalSpeedWalk(const FInputActionValue& InputValue);
	///

	void FirePressed(const FInputActionValue& InputValue);

	void FireReleased(const FInputActionValue& InputValue);

	void InputAimingPressed(const FInputActionValue& InputValue);

	void InputAimingReleased(const FInputActionValue& InputValue);

	void CrouchButtonPressed(const FInputActionValue& InputValue);
	//��G��������
	void InputDropWeapon();

	//��R��������
	void InputReload(const FInputActionValue& InputValue);

	//��V�л��ӽ�
	void InputShiftView(const FInputActionValue& InputValue);

	//��V�л��ӽ�
	void EquipButtonPressed(const FInputActionValue& InputValue);
	///

#pragma endregion InputFunction

public:

#pragma region RPC
	UFUNCTION(Client, Reliable)
		void ClientSetName(const FString& Name);

	UFUNCTION(Server, Reliable)
		void	ServerSetPlayerName(const FString& PlayerName);

	//RPC��ɫ����
	UFUNCTION(Server, Reliable)
		void ServerRunSpeed();

	//RPC��ɫ�ָ��ٶ�
	UFUNCTION(Server, Reliable)
		void ServerNormalSpeedWalk();

	//�л������˳ƺ�ͻ�����ת�����˶�
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



	//��ʼ���ý�ɫ����
	UFUNCTION(Server, Reliable)
		void ServerSetMaterial();

	UFUNCTION(Client, Reliable)
		void ClientSetMaterial();

#pragma endregion RPC

protected:
	//��Ϸ�����������
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Input")
		float TurnRateGamepad = 5.f;

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float RunSpeed = 437.5f;//�����ٶ�

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float RunAimSpeed = 337.5f;//������׼�ٶ�

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float WalkSpeed = 210.f;//�����ٶ�

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float WalkAimSpeed = 180.f;//������׼�ٶ�

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float CrouchSpeed = 180.f;//�¶��ٶ�

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "Speed")
		float CrouchAimSpeed = 150.f;//�¶���׼�ٶ�

	UPROPERTY(Replicated)
	bool IsRunning = false;

	void AimOffset(float DeltaTime);
	//��ȡYaw��offset
	void SimProxiesTurn();
	float AO_Yaw;
	float AO_Pitch;
	float	InterpAO_Yaw;
	FRotator StartingAimRotation;
	void TurnInPlace(float DeltaTime);//ת����
	 ETurningInPlace TurningInPlace;//ת���ö������

	UFUNCTION()
	void ReceiveDamage(AActor *DamagedActor, float Damage, const UDamageType * DamageType, class AController* InstigatorController, AActor * DamageCauser);

	void PollInit();//��ʼ�������
	


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
		UMaterialInterface* TransparentMaterial;//͸������

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "EffectMaterials",meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* NormalMaterialUp;//�ϰ�����������

	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "EffectMaterials",meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* NormalMaterialDown;//�°�����������

		//������̫�涯��
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
		float CameraThreshold = 200.f;//�����ֵ

		bool bRotateRootBone;

		float TurnThreshold = 3.5f;//�ﵽת��Ҫ��Ĳ�ֵ�Ƕ�

		FRotator ProxyRotationLastFrame;//��һ֡�Ĵ�����ת
		FRotator ProxyRotation;

		float ProxyYaw;

		float TimeSinceLastMovementReplication;

		float CalculateSpeed();

	//����ֵ
		UPROPERTY(EditAnywhere, Category = "Player Stats")
		float MaxHealth = 100.f;

		UPROPERTY(ReplicatedUsing = OnRep_Health,  EditAnywhere, Category = "Player Stats")
		float Health = 100.f;

		UFUNCTION()
		void OnRep_Health(float LastHealth);

	//����
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

		//�ܽ�Ч��
		FOnTimelineFloat DissolveTrack;//�ܽ�Ч����ʱ����

		UFUNCTION()
		void UpdateDissolveMaterial(float DissolveValue);
		void StartDissolve();

		//������ʱ���Ըı�Ķ�̬����ʵ��
		UPROPERTY(VisibleAnywhere, category = Elim)
		UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;//��̬�ܽ����ʵ��

		//����ͼ�����õĶ�̬����ʵ��
		UPROPERTY(VisibleAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstance;

		//������ɫ
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

		//��̭Ч��
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

	//Ͷ����
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* AttachedGrenade;

	UPROPERTY(EditAnywhere,Category= "Combat")
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	//�Ƿ����Ѿ��˺�
	UPROPERTY(EditAnywhere, Category = "Team")
	bool bTeamDamage =false;

	//�Ѿ��˺�����
	UPROPERTY(EditAnywhere, Category = "Team")
		float TeamDamageRate = 1.f;
#pragma region EnhancedInput
	///��ǿ����
	//����ӳ���
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Context", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputMappingContext> IMC_Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Context", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputMappingContext> IMC_MoveBase;

	//�����ƶ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_MoveForward;

	//�����ƶ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_MoveRight;

	//�����ӽ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_LookUpRate;

	//�����ӽ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction>  IA_LookRightRate;

	//��Ծ
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Jump;


	//����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_FirePressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_FireReleased;

	//����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_AimingPressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_AimingReleased;

	//����shift��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ShiftPressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ShiftReleased;

	//����Ctrl�¶�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_CrouchPressed;

	//����G��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_DropWeapon;

	//����R��������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Reload;

	//����V�л��ӽ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_ChangeView;

	//����Fʰȡ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EnhancedInput | Action", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<UInputAction> IA_Equip;
	///
#pragma endregion EnhancedInput



public:
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();

	bool IsAiming();
	
	bool IsFirstPerson();//��ȡ�Ƿ��ǵ�һ�˳�

	AWeapon* GetEquippedWeapon();

	FVector GetHitTarget() const;

	ECombatState GetCombatState() const;

	bool IsHoldingTheFlag();
	bool IsLocallyReloading();
	ETeam GetTeam();
	void SetHoldingTheFlag(bool bHolding);
	
	FORCEINLINE bool GetIsCrouch() { return bIsCrouched; }//��ȡ�Ƿ����¶׵�
	FORCEINLINE float GetWalkSpeed() { return WalkSpeed; }//�����ٶ�
	FORCEINLINE float GetRunSpeed() { return RunSpeed; }//�����ٶ�
	FORCEINLINE float GetCrouchSpeed() { return CrouchSpeed; }//�¶��ٶ�
	FORCEINLINE float GetRunAimSpeed() { return RunAimSpeed; }//������׼�ٶ�
	FORCEINLINE float GetCrouchAimSpeed() { return CrouchAimSpeed; }//�¶���׼�ٶ�
	FORCEINLINE float GetWalkAimSpeed() { return WalkAimSpeed; }//������׼�ٶ�
	FORCEINLINE float GetIsRunning() { return IsRunning; }//�Ƿ���
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


