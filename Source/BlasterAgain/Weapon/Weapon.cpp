
#include "Weapon.h"

#include "BlasterAgain/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
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
	//先关掉碰撞
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

	if(HasAuthority()) //与HasAuthority()等效
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

#pragma region WeaponState
void AWeapon::OnRep_WeaponState()//处理武器不同状态时客户端武器的物理效果
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetOwner());
	const USkeletalMeshSocket* HandSocket = BlasterCharacter ? BlasterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket")) : nullptr;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		ShowPickupWidget(false);
		break;
	case  EWeaponState::EWS_Equipped :
		ShowPickupWidget(false);
		//关闭模拟物理
		WeaponMesh->SetSimulatePhysics(false);
		//关闭重力
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 关闭碰撞检测
		if (HandSocket && BlasterCharacter)//!!!踩了一天的坑，服务端附加了客户端也得附加
		{
			HandSocket->AttachActor(this, BlasterCharacter->GetMesh()); //在插槽上将武器附加到身体上
		}
		break;
	case EWeaponState::EWS_Dropped :
		ShowPickupWidget(false);
		FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//这个变量是解绑组件的规则，参数意思是自动计算相对转换，以便分离组件维护相同的世界转换。
		WeaponMesh->DetachFromComponent(DetachRules);//不论组件被附加到什么上面都会拆下来，自动解绑被绑在一起的组件
		//开启模拟物理
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetLinearDamping(LinearDamping);
		WeaponMesh->SetAngularDamping(AngularDamping);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 启用碰撞检测和物理模拟
		//开启重力
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

void AWeapon::ServerDropWeapon_Implementation()//这个用来处理Server端武器的掉落效果，客户端的效果由WeaponState触发的OnRep_WeaponState完成
{
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//这个变量是解绑组件的规则，参数意思是自动计算相对转换，以便分离组件维护相同的世界转换。
	WeaponMesh->DetachFromComponent(DetachRules);//不论组件被附加到什么上面都会拆下来，自动解绑被绑在一起的组件
	//开启模拟物理
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetLinearDamping(LinearDamping);
	WeaponMesh->SetAngularDamping(AngularDamping);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 启用碰撞检测和物理模拟
	//开启重力
	WeaponMesh->SetEnableGravity(true);
	
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);//先将此件武器的状态的设为丢弃状态
	ServerDropWeapon();
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
		ShowPickupWidget(false);//无主的仅在服务端执行，客户端靠OnRep_OverlappingWeapon
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




