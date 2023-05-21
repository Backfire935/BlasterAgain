
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//设置同步
	friend class ABlasterCharacter;

	UFUNCTION()
	void EquipWeapon(class AWeapon* WeaponToEquip);

	UFUNCTION()
	void DropWeapon();
	
	UFUNCTION()
	void SwapPrimaryWeapon();//切换到主武器

	UFUNCTION()
	void SwapSecondaryWeapon();//切换到副武器

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
	void OnRep_EquippedWeapon();//装备武器的Rep_Notify

	UFUNCTION()
	void OnRep_SecondaryWeapon();//装备第二把武器的Rep_Notify

	UFUNCTION(Server, Reliable,WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);//服务器开火

	UFUNCTION(NetMulticast,  Reliable)
	void MultiCastFire(const FVector_NetQuantize& TraceHitTarget);//服务器向客户端发起的开火通知

	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>&  TraceHitTargets);//服务器喷子开火

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);//服务器向客户端发起的喷子开火通知

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
	 float BaseWalkSpeed;//基本行走速度

	 UPROPERTY(EditAnywhere)
	 float AimWalkSpeed;//瞄准行走速度
	 
	 UPROPERTY(EditAnywhere)
	 bool bFireButtonPressed;

	//HUD和准星
	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairVelocityFactor;//默认的行走速度准星扩散

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairInAirFactor;//默认的准星在空中的扩散

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairAimFactor;//默认的准星瞄准扩散

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float CrosshairAimTarget = 0.58f;//默认的瞄准状态下准星的扩散

	 UPROPERTY(EditAnywhere, Category = Combat)
		 float CrosshairShootingFactor;//默认的瞄准状态下准星的扩散

	 FVector HitTarget;

	 FHUDPackage HUDPackage;

	 //瞄准时的FOV
	 UPROPERTY( EditAnywhere,Category = Combat)
		 float DefaultFOV;

	 UPROPERTY(EditAnywhere , Category = Combat)
	 float ZoomedFOV = 30.f;

	 float CurrentFOV;

	 UPROPERTY(EditAnywhere, Category = Combat)
	 float ZoomInterpSpeed = 20.f;

	 void InterpFOV(float DeltaTime);

	 //全自动射击
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

	 //当前手持武器的备弹
	 UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	 int32 CarriedAmmo;//备弹

	 UFUNCTION()
		 void OnRep_CarriedAmmo();

	
	 TMap<EWeaponType, int32> CarriedAmmoMap;//Map类型，hash算法无法在local和server上获得相同的结果，故用中间变量进行网络复制而不是TMap类型本身

	 void InitializeCarriedAmmo();

	 UPROPERTY(EditAnywhere)
	 int32	StartingARAmmo = 30;//步枪子弹

	 UPROPERTY(EditAnywhere)
		 int32	StartingRocketAmmo = 0;//火箭筒弹药

	 UPROPERTY(EditAnywhere)
		 int32	StartingPistolAmmo = 0;//手枪弹药

	 UPROPERTY(EditAnywhere)
		 int32	StartingSMGAmmo = 0;//手枪弹药

	 UPROPERTY(EditAnywhere)
		 int32	StartingShotGunAmmo = 0;//喷子弹药

	 UPROPERTY(EditAnywhere)
		 int32	StartingSniperAmmo = 0;//喷子弹药

	 UPROPERTY(EditAnywhere)
		 int32	StartingGrenadeLauncherAmmo = 0;//喷子弹药


	 UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	 ECombatState CombatState = ECombatState::ECS_Unoccupied;//武器的状态，默认是空闲状态

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

	//动画时蹲下的但是客户端的模型还是站着的
	UPROPERTY(ReplicatedUsing= OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;

	UFUNCTION()
	void OnRep_HoldingTheFlag();

public:	

	UPROPERTY()
	AWeapon* TheFlag;

	FORCEINLINE int32 GetGrenades() const { return Grenades; }
		
};
