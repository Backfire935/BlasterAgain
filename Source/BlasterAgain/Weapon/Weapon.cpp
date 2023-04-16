
#include "Weapon.h"

#include "BlasterAgain/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
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

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		ShowPickupWidget(false);
		break;
	case  EWeaponState::EWS_Equipped :
		ShowPickupWidget(false);
		//�ر�����
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // �ر���ײ���
		//�ر�ģ������
		WeaponMesh->SetSimulatePhysics(false);
		break;
	case EWeaponState::EWS_Dropped :
		ShowPickupWidget(false);
		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//��������ǽ������Ĺ��򣬲�����˼���Զ��������ת�����Ա�������ά����ͬ������ת����
		WeaponMesh->DetachFromComponent(DetachRules);//������������ӵ�ʲô���涼����������Զ���󱻰���һ������
		//����ģ������
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetLinearDamping(LinearDamping);
		WeaponMesh->SetAngularDamping(AngularDamping);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // ������ײ��������ģ��
		//��������
		WeaponMesh->SetEnableGravity(true);
		break;
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case  EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EWeaponState::EWS_Dropped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::ServerDropWeapon_Implementation()
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

	SetOwner(nullptr);//��ӵ��������Ϊ��
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);//�Ƚ��˼�������״̬����Ϊ����״̬
	ServerDropWeapon();
}

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




