
#pragma once

#include "CoreMinimal.h"
#include "BlasterAgain/BlasterTypes/CombatState.h"
#include "BlasterAgain/HUD/BlasterHUD.h"
#include "BlasterAgain/Weapon/Projectile.h"
#include "BlasterAgain/Weapon/WeaponTypes.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TraceLength 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTERAGAIN_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//����ͬ��
	friend class ABlasterCharacter;

	UFUNCTION()
	void EquipWeapon(class AWeapon* WeaponToEquip);

	UFUNCTION()
	void DropWeapon();
	
	UFUNCTION()
	void SwapPrimaryWeapon();//�л���������

	UFUNCTION()
	void SwapSecondaryWeapon();//�л���������

	UFUNCTION()
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReload();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable)
	void ShotGunShellReload();

	void JumpToShotGunEnd();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void LauncherGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLuncherGrenade(const FVector_NetQuantize& Target);

	void PickupAmmo(EWeaponType WeaponType,int32 AmmoAmount);

	bool bLocallyReloading = false;
protected:

	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();//װ��������Rep_Notify

	UFUNCTION()
	void OnRep_SecondaryWeapon();//װ���ڶ���������Rep_Notify

	UFUNCTION(Server, Reliable,WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);//����������

	UFUNCTION(NetMulticast,  Reliable)
	void MultiCastFire(const FVector_NetQuantize& TraceHitTarget);//��������ͻ��˷���Ŀ���֪ͨ

	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>&  TraceHitTargets);//���������ӿ���

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);//��������ͻ��˷�������ӿ���֪ͨ

	UFUNCTION()
	void TraceUnderCrosshairs(FHitResult & TraceHitResult);

	UFUNCTION()
	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

	void ThrowGrenade();

	UFUNCTION(Server,Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> GrenadeClass;

	UFUNCTION()
	void DropEquippedWeapon();

	void AttachActorToRightHand(AActor * ActorToAttach);
	void AttachFlagToLeftHand(AWeapon* Flag);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void AttachActorToBackpack(AActor* ActorToAttach);

	UFUNCTION()
	void UpdateCarriedAmmo();

	void PlayEquipWeaponSound(AWeapon * WeaponToEquip);

	void ReloadEmptyWeapon();

	void SelectWeaponSocket(FName SocketName);

	UFUNCTION(BlueprintCallable)
	void ShowAttachedGrenade(bool bShowGrenade);

	UFUNCTION()
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);

	UFUNCTION()
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
private:
	
	class ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY()
	class ABlasterHUD* HUD;
	
	UPROPERTY(Replicated)
	 AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

 bool bAimButtonPressed = false;

	UFUNCTION()
	 void OnRep_Aiming();

	 UPROPERTY(EditAnywhere)
	 float BaseWalkSpeed;//���������ٶ�

	 UPROPERTY(EditAnywhere)
	 float AimWalkSpeed;//��׼�����ٶ�
	 
	 UPROPERTY(EditAnywhere)
	 bool bFireButtonPressed;

	//HUD��׼��
	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairVelocityFactor;//Ĭ�ϵ������ٶ�׼����ɢ

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairInAirFactor;//Ĭ�ϵ�׼���ڿ��е���ɢ

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairAimFactor;//Ĭ�ϵ�׼����׼��ɢ

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairAimTarget = 0.58f;//Ĭ�ϵ���׼״̬��׼�ǵ���ɢ

	 UPROPERTY(EditAnywhere, Category = Combat)
		 float CrosshairShootingFactor;//Ĭ�ϵ���׼״̬��׼�ǵ���ɢ

	 FVector HitTarget;

	 FHUDPackage HUDPackage;

	 //��׼ʱ��FOV
	 UPROPERTY( EditAnywhere,Category = Combat)
		 float DefaultFOV;

	 UPROPERTY(EditAnywhere , Category = Combat)
	 float ZoomedFOV = 30.f;

	 float CurrentFOV;

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float ZoomInterpSpeed = 20.f;

	 void InterpFOV(float DeltaTime);

	 //ȫ�Զ����
	 FTimerHandle FireTimer;

	 bool bCanFire = true;

	 void StartFireTimer();
	 void FireTimerFinished();
	 void Fire();

	 UFUNCTION()
	 void FireProjectileWeapon();

	 UFUNCTION()
	 void FireHitScanWeapon();

	 UFUNCTION()
	 void FireShotgun();

	 UFUNCTION()
	 void LocalFire(const FVector_NetQuantize & TraceHitTarget);

	 UFUNCTION()
	 void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget);


	 bool CanFire();

	 //��ǰ�ֳ������ı���
	 UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	 int32 CarriedAmmo;//����

	 UFUNCTION()
		 void OnRep_CarriedAmmo();

	
	 TMap<EWeaponType, int32> CarriedAmmoMap;//Map���ͣ�hash�㷨�޷���local��server�ϻ����ͬ�Ľ���������м�����������縴�ƶ�����TMap���ͱ���

	 void InitializeCarriedAmmo();

	 UPROPERTY(EditAnywhere)
	 int32	StartingARAmmo = 30;//��ǹ�ӵ�

	 UPROPERTY(EditAnywhere)
		 int32	StartingRocketAmmo = 0;//���Ͳ��ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingPistolAmmo = 0;//��ǹ��ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingSMGAmmo = 0;//��ǹ��ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingShotGunAmmo = 0;//���ӵ�ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingSniperAmmo = 0;//���ӵ�ҩ

	 UPROPERTY(EditAnywhere)
		 int32	StartingGrenadeLauncherAmmo = 0;//���ӵ�ҩ


	 UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	 ECombatState CombatState = ECombatState::ECS_Unoccupied;//������״̬��Ĭ���ǿ���״̬

	 UFUNCTION()
	 void OnRep_CombatState();

	 UFUNCTION()
	 void UpdateAmmoValues();

	 UFUNCTION()
	 void UpdateShotgunAmmoValues();

	UPROPERTY(ReplicatedUsing=OnRep_Grenades,EditAnywhere)
	 int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	 int32 MaxGrenades = 9;

	void UpdateHUDGrenades();

	//����ʱ���µĵ��ǿͻ��˵�ģ�ͻ���վ�ŵ�
	UPROPERTY(ReplicatedUsing= OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;

	UFUNCTION()
	void OnRep_HoldingTheFlag();

public:	

	UPROPERTY()
	AWeapon* TheFlag;

	FORCEINLINE int32 GetGrenades() const { return Grenades; }
		
};
