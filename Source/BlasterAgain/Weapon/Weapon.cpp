
#include "Weapon.h"

#include "WeaponTypes.h"
#include "BlasterAgain/BlasterComponent/CombatComponent.h"
#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/PlayerController/BlasterPlayerController.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


AWeapon::AWeapon()
{
 	
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	//WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//�ȹص���ײ
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AreaSphere->SetSphereRadius(72.f);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
	
}


void AWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);//������������Ⱦ�Զ������
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority()) //��HasAuthority()��Ч
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
}



void AWeapon::HandleWeaponEquiped()
{
	ShowPickupWidget(false);//�ر�������ʰȡ��ʾ
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�ر���������ײ����
	WeaponMesh->SetSimulatePhysics(false);//����ģ������
	WeaponMesh->SetEnableGravity(false);//������������
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)//����ǹ����ģ������
		{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);//������������
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
	EnableCustomDepth(false);//�ر������������

	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && bUseServerSideRewind)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && !BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::HandleWeaponSecondary()
{
	ShowPickupWidget(false);//�ر�������ʰȡ��ʾ
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�ر���������ײ����
	WeaponMesh->SetSimulatePhysics(false);//����ģ������
	WeaponMesh->SetEnableGravity(false);//������������
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)//����ǹ����ģ������
		{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);//������������
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
	EnableCustomDepth(true);//�ر������������
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();//������ǵ�ǰ����Ⱦ״̬Ϊ����Ⱦ
	}
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;

	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}


void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

#pragma region WeaponState


void AWeapon::OnRep_WeaponState()//����������ͬ״̬ʱ�ͻ�������������Ч��
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetOwner());
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//��������ǽ������Ĺ��򣬲�����˼���Զ��������ת�����Ա�������ά����ͬ������ת����
	const USkeletalMeshSocket* HandSocket = BlasterCharacter ? BlasterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket")) : nullptr;
	switch (WeaponState)
	{
	case EWeaponState::Weapon_Initial:
		ShowPickupWidget(false);
		break;
		
	case  EWeaponState::Weapon_Equipped :
		ShowPickupWidget(false);
		//�ر�ģ������
		WeaponMesh->SetSimulatePhysics(false);
		//�ر�����
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // �ر���ײ���
		if (HandSocket && BlasterCharacter)//!!!����һ��Ŀӣ�����˸����˿ͻ���Ҳ�ø���
		{
			HandSocket->AttachActor(this, BlasterCharacter->GetMesh()); //�ڲ���Ͻ��������ӵ�������
		}
		break;
		
	case EWeaponState::Weapon_Dropped :
		ShowPickupWidget(false);
		WeaponMesh->DetachFromComponent(DetachRules);//������������ӵ�ʲô���涼����������Զ���󱻰���һ������
		//����ģ������
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetLinearDamping(LinearDamping);
		WeaponMesh->SetAngularDamping(AngularDamping);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // ������ײ��������ģ��
		//��������
		WeaponMesh->SetEnableGravity(true);
		break;
		
	case EWeaponState::Weapon_EquippedSecondary :
		{
			HandleWeaponSecondary();
			break;
		}
		break;
	}
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombat() && IsFull())
	{
		BlasterOwnerCharacter->GetCombat()->JumpToShotGunEnd();
	}

}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1 , 0 , MagCapacity);//��ammo������0����󱸵���֮��
	SetHUDAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else  
	{
		++Sequence;
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
	case EWeaponState::Weapon_Initial:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case  EWeaponState::Weapon_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::Weapon_Dropped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

bool AWeapon::IsEmpty()//�ж��ӵ��Ƿ����
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;//��ǰʣ���ӵ������ǲ��ǵ��ڵ�������
}

void AWeapon::ServerDropWeapon_Implementation()//�����������Server�������ĵ���Ч�����ͻ��˵�Ч����WeaponState������OnRep_WeaponState���
{
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//��������ǽ������Ĺ��򣬲�����˼���Զ��������ת�����Ա�������ά����ͬ������ת����
	WeaponMesh->DetachFromComponent(DetachRules);//������������ӵ�ʲô���涼����������Զ���󱻰���һ������
	//����ģ������
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetLinearDamping(LinearDamping);
	WeaponMesh->SetAngularDamping(AngularDamping);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // ������ײ��������ģ��
	//��������
	WeaponMesh->SetEnableGravity(true);
	
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::Weapon_Dropped);//�Ƚ��˼�������״̬����Ϊ����״̬
	ServerDropWeapon();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : BlasterOwnerCharacter;
		if(BlasterOwnerCharacter && BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{//ȷ��Ŀǰ�õĵ�ҩHUD�������õ������ĵ�ҩHUD�����Ǳ��ϱ���
			SetHUDAmmo();
		}
	}
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	//�������������Ч��
	if (FireAnimation)//������𶯻����ڵĻ�,������ģ�Ͳ���������𶯻�
		{
			WeaponMesh->PlayAnimation(FireAnimation, false);//�ڶ���������ѭ�����ŵĲ���������Ϊfalse
		}

	//�������ɲ��׳��ӵ���
	if (BulletShells)//����ӵ������Ǵ��ڵĻ�
		{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));//��ȡ����ǹ�ڵĲ��
		if (AmmoEjectSocket)//ǹ�ڲ�۵�λ��
			{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);//��ȡ��ǰ���ϵ�������ǹ�ڲ�۵�λ�ú���ת��Ϣ��������ӵ����׳��ķ��򣬾�����Ҫ��ǹе�Ĺ��������ò��λ�ú���ת�Ƕ�
			UWorld* World = GetWorld();//��ȡ���糡��
			if (World)
			{
				World->SpawnActor<ABulletShells>(//ֱ�������糡���������ӵ��ǣ������Ǿ�������
					BulletShells,//���ɵ��������ӵ���
					SocketTransform.GetLocation(),//���ɵ�λ����������۵�λ��
					SocketTransform.GetRotation().Rotator()	//���ɵ���ת�Ƕ��ǲ�۵���ά��ת��Ϣ
					);
			}
			}
		}//�ӵ����׳�Ч������

	SpendRound();//�����˸������ӵ�
}

FVector AWeapon::TraceWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket == nullptr) return FVector();//һ��ֱ�������������

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();//���������ʼ��

	const FVector  ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//һ����������ʼ�㵽������Ŀ�������
	const FVector  SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;//����������յ���е�����
	const FVector  RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);//�������λ����*������� 
	const FVector EndLoc = SphereCenter + RandVec;//���ĵ����ܵ������ɢ����
	const FVector ToEndLoc = EndLoc - TraceStart;//�������߶�

	FVector EndEnd = (TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());//������������

	/*
	 *DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, false, 10.f);//����ɢ����ɢ��Χ
	DrawDebugSphere(GetWorld(), EndLoc, 4.F, 12, FColor::Blue, false,10.f);//���������ӵ������
	DrawDebugLine(GetWorld(), TraceStart, EndEnd, FColor::Orange, false, 10.f);//���������ӵ������
*/

	return EndEnd;//������������
}

void AWeapon::ReadyDestroyWeapon()
{
	GetWorldTimerManager().SetTimer(
	DestroyWeaponTimer,
	this,
	&AWeapon::DestroyWeapon,
	DestroyWeaponTime
);
}

void AWeapon::DestroyWeapon()
{
	//���֮ǰ���������������Ļ�����ôEquippedWeapon��StoryedToDestroyWeaponָ��ָ�����ͬһ��ʵ������
	if (bDestroyedWeapon)//������ǰ����һ�μ�飬����ʱ�����Ѿ���ʰȡ�������򲻻���������
		{
			Destroy();
		}
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

#pragma endregion WeaponState

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		ShowPickupWidget(false);//�����Ľ��ڷ����ִ�У��ͻ��˿�OnRep_OverlappingWeapon
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

