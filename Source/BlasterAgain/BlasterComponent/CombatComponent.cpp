
#include "CombatComponent.h"

#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/HUD/BlasterHUD.h"
#include "BlasterAgain/Interfaces/InteractWithCrosshairsInterface.h"
#include "BlasterAgain/PlayerController/BlasterPlayerController.h"
#include "BlasterAgain/PlayerController/FPSAimCamera.h"
#include "BlasterAgain/Weapon/ShotGun.h"
#include "BlasterAgain/Weapon/Weapon.h"
#include "BlasterAgain/Weapon/WeaponTypes.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;//�����ٶ�
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;//��ȡ�����Ұ
			CurrentFOV = DefaultFOV;
		}
		if (Character->HasAuthority())//���Ʊ���
			{
				InitializeCarriedAmmo();//��ʼ������
			}

		
	}
}

#pragma region Aiming
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bAiming);
	if(Character && bAiming)//�Ƿ�����׼
	{
		if(Character->GetIsCrouch())//�Ƿ����¶�
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchAimSpeed();//���ó��¶���׼���ٶ�
		}
		else//վ��
		{
				Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunAimSpeed() : Character->GetWalkAimSpeed();//�ǲ����ڱ��� �����óɱ�����׼���ٶ�  �������ó�������׼���ٶ�
		}
	}
	else if(Character && !bAiming)//������׼��
	{
		if (Character->GetIsCrouch())//���¶�
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchSpeed();//�ָ����¶׵������ٶ�
		}
		else//վ��
		{
				Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunSpeed() : Character->GetWalkSpeed();//�ǲ����ڱ��ܣ������óɱ��ܵ��ٶ� �������ó����ߵ��ٶ�
		}
	}
}



void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character && bAiming)//�Ƿ�����׼
	{
		if (Character->GetIsCrouch())//�Ƿ����¶�
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchAimSpeed();//���ó��¶���׼���ٶ�
		}
		else//վ��
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunAimSpeed() : Character->GetWalkAimSpeed();//�ǲ����ڱ��� �����óɱ�����׼���ٶ�  �������ó�������׼���ٶ�
		}
	}
	else if (Character && !bAiming)//������׼��
	{
		if (Character->GetIsCrouch())//���¶�
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Character->GetCrouchSpeed();//�ָ����¶׵������ٶ�
		}
		else//վ��
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = Character->GetIsRunning() ? Character->GetRunSpeed() : Character->GetWalkSpeed();//�ǲ����ڱ��ܣ������óɱ��ܵ��ٶ� �������ó����ߵ��ٶ�
		}
	}
}
#pragma endregion Aiming

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		SetHUDCrosshairs(DeltaTime); 
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);//ע��ڶ�������
	DOREPLIFETIME(UCombatComponent, CombatState);

	DOREPLIFETIME(UCombatComponent, Grenades);

	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly); //ע�᱾����������int32���͵ı���������������ͬ��

	DOREPLIFETIME(UCombatComponent, bHoldingTheFlag);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);
	//�ر�ģ������
	EquippedWeapon->GetWeaponMesh()->SetSimulatePhysics(false);
	//�ر�����
	EquippedWeapon->GetWeaponMesh()->SetEnableGravity(false);
	EquippedWeapon->GetWeaponMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // �ر���ײ���

	const USkeletalMeshSocket * HandSocket =  Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh()); //�ڲ���Ͻ��������ӵ�������
	}
	EquippedWeapon->SetOwner(Character);
	bCanFire = true;
	bLocallyReloading = false;
	
	UpdateAmmoValues();
	UpdateCarriedAmmo();
	
	//Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	//Character->bUseControllerRotationYaw = true;
	
	/*if(Character->GetAimCamera())
	{
		Character->GetAimCamera()->AttachToComponent(EquippedWeapon->GetWeaponMesh(),FAttachmentTransformRules::SnapToTargetIncludingScale,FName(TEXT("AimSocket")));
		UE_LOG(LogTemp,Warning,TEXT("GetAimActor = %.0f,%.0f,%.0f"),
			Character->GetAimCamera()->GetActorLocation().X,
			Character->GetAimCamera()->GetActorLocation().Y,
			Character->GetAimCamera()->GetActorLocation().Z);
	}*/
}



void UCombatComponent::DropWeapon()
{
	if (EquippedWeapon)//�����������
	{
		EquippedWeapon->Dropped();//����Ѿ�װ����һ���������Ͷ������ϵ�
		EquippedWeapon->SetOwner(nullptr);//��ӵ��������Ϊ��
		EquippedWeapon = nullptr;
		
	}
}

void UCombatComponent::SwapPrimaryWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	//�����������Ĳ���,�������ͳ������������
	EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);//����������״̬Ϊ��װ��
	AttachActorToRightHand(EquippedWeapon);//������������
	EquippedWeapon->SetOwner(Character);//��������Ȩ
	EquippedWeapon->SetHUDAmmo();//���õ�ǰ��ҩHUD
	UpdateAmmoValues();
	UpdateCarriedAmmo();//����Я���ĵ�ҩ
	PlayEquipWeaponSound(EquippedWeapon);//���ż�������������
	ReloadEmptyWeapon();//����Ҫ�ǿյľ�װ�ӵ�
	//���ø������Ĳ���

	SecondaryWeapon->SetWeaponState(EWeaponState::Weapon_EquippedSecondary);//����������״̬Ϊ��װ��
	AttachActorToBackpack(SecondaryWeapon);//�����ŵ���ȥ
}

void UCombatComponent::SwapSecondaryWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	AWeapon* TempWeapon = SecondaryWeapon;
	SecondaryWeapon = EquippedWeapon;
	EquippedWeapon = TempWeapon;

	//�����������Ĳ���,�������ͳ������������
	EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);//����������״̬Ϊ��װ��
	AttachActorToRightHand(EquippedWeapon);//������������
	EquippedWeapon->SetOwner(Character);//��������Ȩ
	EquippedWeapon->SetHUDAmmo();//���õ�ǰ��ҩHUD
	UpdateCarriedAmmo();//����Я���ĵ�ҩ
	PlayEquipWeaponSound(EquippedWeapon);//���ż�������������
	ReloadEmptyWeapon();//����Ҫ�ǿյľ�װ�ӵ�
	//���ø������Ĳ���

	SecondaryWeapon->SetWeaponState(EWeaponState::Weapon_EquippedSecondary);//����������״̬Ϊ��װ��
	AttachActorToBackpack(SecondaryWeapon);//�������ŵ���ȥ
}

void UCombatComponent::Reload()
{
	UE_LOG(LogTemp,Warning,TEXT("%d"),CarriedAmmo);
	UE_LOG(LogTemp,Warning,TEXT("%d"),CombatState);
	UE_LOG(LogTemp,Warning,TEXT("%d"),EquippedWeapon->IsFull());
	UE_LOG(LogTemp,Warning,TEXT("%d"),bLocallyReloading);
	
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading && !EquippedWeapon->IsFull() && !bLocallyReloading)//�����ӵ��Ҳ��������ҵ�ǰ����״̬�����ڻ������ܹ�����
		{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
		
		}
}

void UCombatComponent::FinishReload()
{
	if (Character == nullptr)  return;
	bLocallyReloading = false;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
		UpdateCarriedAmmo();
		UE_LOG(LogTemp, Warning, TEXT("FinishReload"));
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed && EquippedWeapon && !bLocallyReloading)//������Ӽ�������ᵼ�µ�����ڵ�ʱ����Ϊû������������bPressed=false���޷�����
		{
			Fire();
		}
}

void UCombatComponent::ShotGunShellReload()
{
	if(Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();//ÿ��+1�ӵ�
	}
}

void UCombatComponent::JumpToShotGunEnd()
{
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotGunEnd"));//ֱ����ת���ƶ�����̫�涯��
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LauncherGrenade()
{
	if(Character && Character->IsLocallyControlled())
	{
		ServerLuncherGrenade(HitTarget);
	}
}

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))//ȷ�����������������͵�
		{
		CarriedAmmoMap[WeaponType] += AmmoAmount;//��ָ�����������ͼӱ���

		UpdateCarriedAmmo();
		}
	if(EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();//�������û�ӵ����Ҵ�ʱ�����ӵ����Զ�װ��
	}
}

void UCombatComponent::WeaponRecoil()
{
	UCurveFloat* VerticalRecoilCurve = nullptr;
	UCurveFloat* HorizontalRecoilCurve = nullptr;
	if(EquippedWeapon->VerticalRecoil)
	{
		VerticalRecoilCurve = EquippedWeapon->VerticalRecoil;
		EquippedWeapon->RecoilXCoordPerShoot += 0.1;
		EquippedWeapon->NewVerticalRecoilValue = VerticalRecoilCurve->GetFloatValue(EquippedWeapon->RecoilXCoordPerShoot);

		EquippedWeapon->VerticalRecoilAmount = EquippedWeapon->NewVerticalRecoilValue - EquippedWeapon->OldVerticalRecoilValue;;
		EquippedWeapon->OldVerticalRecoilValue = EquippedWeapon->NewVerticalRecoilValue;
	}
	
	if(EquippedWeapon->HorizontalRecoil)
	{
		HorizontalRecoilCurve = EquippedWeapon->HorizontalRecoil;
		EquippedWeapon->RecoilYCoordPerShoot += 0.1;
		EquippedWeapon->NewHorizontalRecoilValue = HorizontalRecoilCurve->GetFloatValue(EquippedWeapon->RecoilYCoordPerShoot);

		EquippedWeapon->HorizontalRecoilAmount = EquippedWeapon->NewHorizontalRecoilValue - EquippedWeapon->OldHorizontalRecoilValue;;
		
		EquippedWeapon->OldHorizontalRecoilValue = EquippedWeapon->NewHorizontalRecoilValue;
	}

	if(Controller)
	{
		FRotator ControllerRotator = Controller->GetControlRotation();
		Controller->SetControlRotation(FRotator(
			ControllerRotator.Pitch + EquippedWeapon->VerticalRecoilAmount,
			ControllerRotator.Yaw + EquippedWeapon->HorizontalRecoilAmount,
				ControllerRotator.Roll));
	}
	
}

void UCombatComponent::ServerLuncherGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character  && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Character;
		SpawnParameters.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParameters
				);
		}
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);

		AttachActorToRightHand(EquippedWeapon);//������������

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;//�رս�ɫ���ƶ��ķ�����ת
		Character->bUseControllerRotationYaw = true; //������ɫ��������������ת

		PlayEquipWeaponSound(EquippedWeapon);//����װ����������Ч
		EquippedWeapon->EnableCustomDepth(false);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if(SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::Weapon_EquippedSecondary);
		AttachActorToBackpack(SecondaryWeapon);//�ѵڶ�������װ�󱳰���
		PlayEquipWeaponSound(SecondaryWeapon);//����װ����������Ч
		SecondaryWeapon->EnableCustomDepth(true);
	
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MultiCastFire(TraceHitTarget);//�ڷ�������ִ�жಥ���ڷ����������еĿͻ�����ִ��
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}


void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;//����ӵ�֪ͨ���Ǳ������Ͳ�ִ����仰����Ϊ֮ǰִ�й���
	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;//����ӵ�֪ͨ���Ǳ������Ͳ�ִ����仰����Ϊ֮ǰִ�й���
	LocalShotgunFire(TraceHitTargets);
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)//��GEngine��ȡ�ӿ�
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);//��ȡ��Ϸ��Ļ�Ĵ�С
		}
	FVector2D CrosshairLocation(ViewportSize.X / 2.0F, ViewportSize.Y / 2.0f);//��Ļ��С���Զ��������ĵ������,��������Ļ���꣬������������

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//��������2D��Ļ�ռ�����ת��Ϊ��ά����ռ��ͷ���
	bool bScreenToWorld = 	UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);//���к�rosshairWorldPosition,CrosshairWorldDirection���������
	
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;//�ӵ����ȥ��ʼ��λ��

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
			//DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red, false);
		}

		FVector End = Start + CrosshairWorldDirection * TraceLength;//�ӵ����ȥֹͣ��λ�ã��൱��������

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (!TraceHitResult.bBlockingHit) TraceHitResult.ImpactPoint = End;
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())//������߼���Ƿ���ײ�������岢���Ƿ�̳��˽ӿ�
			{
			HUDPackage.CrosshairsColor = FLinearColor::Red;//��������׼�ı��
			}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;//û������׼�ı��
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
		if (Character == nullptr || Character->Controller == nullptr) return;//���ж���ɫ�治����

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)//���жϿ������治����
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;

		if (HUD)//���ж�HUD�治����
		{
			if (EquippedWeapon)//���ж��Ƿ�װ��������
			{//Ҫ�Ǵ���HUD��ʹ�װ�������������õ�Ԥ���׼��
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{//Ҫ��û��������գ���û��׼����
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			//����һ���ٶ����䵽��ɢ����ӳ��[0,600]->[0,1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityOutputRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityOutputRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			//����׼����ɢ����
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, CrosshairAimTarget, DeltaTime, 30.f);//��׼��ʱ��׼�Ǳ�С��׼
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);//û��׼��ʱ��׼����ɢΪ0(�����ǿ�ǹ�Ϳ���)
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);//����׼׼����ɢֵ���ٻָ���0

			HUDPackage.CrosshairsSpread =
				0.5 +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;
			HUD->SetHUDPackage(HUDPackage);

		}
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;//�л�����״̬������
	if(!Character->IsLocallyControlled()) HandleReload();//�����Ѿ����ù�һ�λ��������������֪ͨ��������
}

void UCombatComponent::HandleReload()
{
	if(Character)
	{
		//��������������̫��
		if(EquippedWeapon)
		{
			EquippedWeapon->PlayReloadAnim();
		}
		//���Ž�ɫ������̫��
		Character->PlayReloadMontage();
	}
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();//�����е����¿ռ� = ���� - ʣ���ӵ�

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//�����������
		{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountCarried);//ȡRoomInMag��AmountCarried֮����С�ı���
		return FMath::Clamp(RoomInMag, 0, Least);//ʹRoomInMag
		}
	return 0;
}

void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;
	if (Character && Character->GetAttachedGrenade())
	{
		ShowAttachedGrenade(true);
		CombatState = ECombatState::ECS_ThrowingGrenade;
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
	}
	if(Character && !Character->HasAuthority() && Character->GetAttachedGrenade())
	{
		ServerThrowGrenade();
	}
	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);//����������������ļ���
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if(Character)
	{
		ShowAttachedGrenade(true);
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);//����������������ļ���
	UpdateHUDGrenades();
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)//�����������
		{
		EquippedWeapon->Dropped();//����Ѿ�װ����һ���������Ͷ������ϵ�
		}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));//��ȡ��ɫģ�����ϵ�ָ�����
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());//��������������ɫģ��
	}
}

void UCombatComponent::AttachFlagToLeftHand(AWeapon* Flag)
{
	if (Character == nullptr || Flag == nullptr || Character->GetMesh() == nullptr) return;

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("FlagSocket"));//��ȡ��ɫģ�����ϵ�ָ�����
	if (HandSocket)
	{
		HandSocket->AttachActor(Flag, Character->GetMesh());//��������������ɫģ��
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr || EquippedWeapon == nullptr) return;
	
	switch (EquippedWeapon->GetWeaponType())
	{
	case  EWeaponType::EWT_Pistol :
		{
			SelectWeaponSocket(FName("PistolSocket"));
			break;
		}

	case  EWeaponType::EWT_SMG :
		{
			SelectWeaponSocket(FName("PistolSocket"));
			break;
		}

	default:
		{
			SelectWeaponSocket(FName("LeftHandSocket"));
			break;
		}

	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));//��ȡ��ɫ�󱳱����Ĳ��
	if(BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//�����������
		{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];//��map�л�ȡ��ǰ��������ı�����
		}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);//���ñ���HUD
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip ==nullptr || WeaponToEquip->EquipSound == nullptr) return;
	//����װ����������Ч
	UGameplayStatics::PlaySoundAtLocation(
		this,
		WeaponToEquip->EquipSound,
		Character->GetActorLocation()
	);
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (Character && EquippedWeapon && EquippedWeapon->IsEmpty())//����������������ǿյľ��Զ�����
		{
		Reload();
		}

}

void UCombatComponent::SelectWeaponSocket(FName SocketName)
{
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName(SocketName));//��ȡ��ɫģ�����ϵ�ָ�����
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());//��������������ɫģ��
	}
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if(Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon();//��������ж����Ͱ����ϵĶ�������
	EquippedWeapon = WeaponToEquip;//װ������������Ϊ���������
	EquippedWeapon->SetWeaponState(EWeaponState::Weapon_Equipped);//����������״̬Ϊ��װ��
	EquippedWeapon->bDestroyedWeapon = false;//�������������䵽���Ϻ�׼�����ٵļ�ʱ
	AttachActorToRightHand(EquippedWeapon);//������������
	EquippedWeapon->SetOwner(Character);//��������Ȩ
	EquippedWeapon->SetHUDAmmo();//���õ�ǰ��ҩHUD
	UpdateCarriedAmmo();//����Я���ĵ�ҩ
	PlayEquipWeaponSound(WeaponToEquip);//���ż�������������
	ReloadEmptyWeapon();//����Ҫ�ǿյľ�װ�ӵ�
	EquippedWeapon->EnableCustomDepth(false);//�ر������������
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip;//װ������������Ϊ���������
	SecondaryWeapon->bDestroyedWeapon = false;//��ʼ���򲻽����������䵽���Ϻ�׼�����ٵļ�ʱ
	SecondaryWeapon->SetWeaponState(EWeaponState::Weapon_EquippedSecondary);//����������״̬Ϊ��װ��
	AttachActorToBackpack(WeaponToEquip);
	PlayEquipWeaponSound(WeaponToEquip);//���ż�������������
	SecondaryWeapon->EnableCustomDepth(true);//���������������
	SecondaryWeapon->SetOwner(Character);//��������Ȩ
}

void UCombatComponent::OnRep_Aiming()
{
	if(Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());//�Դ�����׼״̬��FOV��������
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);//�Է���׼״̬��FOV��������
		
		/*if (Character && Character->GetFollowCamera())
		{
			Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);//����Ϊ��ǰ��FOV
		}*/
	}
	
	
	
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr ) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutoMatic)//������ﲻ����ȫ�Զ����ڶ��λ�ֱ��ͣ������
		{
		Fire();
		}

	ReloadEmptyWeapon();
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;

		if (EquippedWeapon)//���ӿ���׼��ɢ��
			{
			CrosshairShootingFactor = 0.75f;

			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				{
					FireProjectileWeapon();
					break;
				}
			case EFireType::EFT_HitScan:
				{
					FireHitScanWeapon();
					break;
				}
			case EFireType::EFT_Shotgun:
				{
					FireShotgun();
					break;
				}

			default:
				break;
			}

			}
		StartFireTimer();
	}	
}

void UCombatComponent::FireProjectileWeapon()
{
	if(EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceWithScatter(HitTarget) : HitTarget;
		if(!Character->HasAuthority()) LocalFire(HitTarget);//��ֹ����˵������Σ�һ��localfire��һ��serverfire
		ServerFire(HitTarget,EquippedWeapon->FireDelay);
	}

}

void UCombatComponent::FireHitScanWeapon()
{
	if(EquippedWeapon && Character)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority()) LocalFire(HitTarget);//��ֹ����˵������Σ�һ��localfire��һ��serverfire
		ServerFire(HitTarget,EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireShotgun()
{
	AShotGun* ShotGun = Cast<AShotGun>(EquippedWeapon);//����������ת��Ϊ����
	if(ShotGun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets;
		ShotGun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);//��ȡ�����ӵ�Ⱥ�����
		if (!Character->HasAuthority()) LocalShotgunFire(HitTargets);//��ֹ����˵������Σ�һ��localfire��һ��serverfire
		ServerShotgunFire(HitTargets);
	}
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;

	if (Character && CombatState == ECombatState::ECS_Unoccupied)//�����ɫ����ڣ�������״̬�ǿ���״̬�Ļ�
		{
			//������Ҫ�������ֶ�����һ���ǽ�ɫ�Ŀ���������������һ���������Ķ�������
			Character->PlayFireMontage(bAiming);//��ɫ�ಥ�ſ�����̫�涯��
			EquippedWeapon->Fire(TraceHitTarget);//�����ഥ�������¼�
			if(Controller && EquippedWeapon->CameraShakeClass)
			{
				Controller->PlayerCameraShake(EquippedWeapon->CameraShakeClass);
			}
			WeaponRecoil();
		}
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
	AShotGun* ShotGun = Cast<AShotGun>(EquippedWeapon);//����������ת��Ϊ����
	if(ShotGun == nullptr || Character == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);//��ɫ�ಥ�ſ�����̫�涯��
		ShotGun->FireShotgun(TraceHitTarget);
		CombatState = ECombatState::ECS_Unoccupied;//������״̬��������Ϊ ��ͨ״̬
		if(Controller && EquippedWeapon->CameraShakeClass)
		{
			Controller->PlayerCameraShake(EquippedWeapon->CameraShakeClass);
		}
			WeaponRecoil();
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	if (bLocallyReloading) return false;//���ؼ��һ���Ƿ��ڻ���������������
	if(!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun)//����Ϊ���Ӳ����ڻ����������
		{
		return true;//������
		}
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;//�ӵ���Ϊ0��������
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);//���ñ���HUD
	}
	bool bJumpToShotGunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun &&
		CarriedAmmo == 0;
	if(bJumpToShotGunEnd)
	{
		//���װ���˾�ֱ������װ����ϵĶ���
		JumpToShotGunEnd();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Sniper, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			ShowAttachedGrenade(true);
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
		}
		break;
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//�����������
		{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);//���ñ���HUD
	}

	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//�����������
		{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		}

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);//���ñ���HUD
	}
	EquippedWeapon->AddAmmo(1);//һ�������ӵ����϶����ܴ���
	bCanFire = true;

	
	if(EquippedWeapon->IsFull() || CarriedAmmo == 0 )
	{
		//���װ���˾�ֱ������װ����ϵĶ���
		JumpToShotGunEnd();
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);

	}
}

void UCombatComponent::OnRep_HoldingTheFlag()
{
	if(bHoldingTheFlag && Character && Character->IsLocallyControlled())
	{
		Character->Crouch();
	}
}

