// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BulletShells.h"
#include "WeaponTypes.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	Weapon_Initial UMETA(DisplayName = "Initial State"),
	Weapon_Equipped UMETA(DisplayName = "Equipped"),
	Weapon_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	Weapon_Dropped UMETA(DisplayName = "Dropped"),
	Weapon_MAX UMETA(DisplayName = "DefaultMax")

};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	EFT_MAX UMETA(DisplayName = "DefaultMax")

};

UCLASS()
class BLASTERAGAIN_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//����ͬ��
	void ShowPickupWidget(bool bShowWidget);
	virtual void Dropped();

	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	virtual void Fire(const FVector &HitTarget);
	FVector TraceWithScatter(const FVector& HitTarget);//����ɢ������߼��
	UFUNCTION()
	void ReadyDestroyWeapon();
	UFUNCTION()
	void DestroyWeapon();
	
	FTimerHandle DestroyWeaponTimer;
	//bool bIsDroppedToDestroyed = false;//�����Ƿ񱻶������ϵȴ�����
	UPROPERTY(EditAnywhere, Category = "Combat")
	float DestroyWeaponTime;
	void AddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere, category = Combat)
	float FireDelay = 0.15f;//�Զ�����Ĭ�Ͽ�����

	UPROPERTY(EditAnywhere, category = Combat)
	bool bAutoMatic = true;//Ĭ����������ģʽ:ȫ�Զ�

	/*
����׼�ŵ�����
*/
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	//��׼ʱ��ҰFOV�仯
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.F;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;//װ������ʱ����Ч

	//�����������Զ�����Ⱦ���
	void EnableCustomDepth(bool bEnable);

	bool bDestroyedWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;//�Ƿ������ӵ�ɢ��
	
protected:
	
	virtual void BeginPlay() override;
	
	
	virtual void HandleWeaponEquiped();//����װ����������
	virtual void HandleWeaponSecondary();//����ڶ�������
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
		 void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//���߽�����λ�÷����ӵ�
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;//���ӵ���Ч��������

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;//��ɢ��Χ

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	UPROPERTY(Replicated,EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;

	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing= OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;
	UPROPERTY(EditAnywhere)
	TSubclassOf<ABulletShells> BulletShells;

	
	UPROPERTY(EditAnywhere)
	int32 Ammo;//�����������ӵ�����

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	UPROPERTY(EditAnywhere)
	int32 MagCapacity = 30;//��������

	int32 Sequence = 0;//���ӵ������йص�δ���������������������	

	UFUNCTION()
	void SpendRound();//�����˸������ӵ�
	
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		class UWidgetComponent* PickupWidget;


	UPROPERTY(EditAnywhere, Category = "Physics")
		float LinearDamping = 0.1f; // ��������ϵ��

	UPROPERTY(EditAnywhere, Category = "Physics")
		float AngularDamping = 0.1f; // ������ϵ��

	
public:
	void SetWeaponState(EWeaponState State);

	UFUNCTION(Server,Reliable)
		void ServerDropWeapon();

	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();//�ж��ӵ��Ƿ����
	bool IsFull();//�ж��ӵ��Ƿ�������
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; };
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	
};



 


