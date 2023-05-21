// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"

#include "BlasterAgain/Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

AFlag::AFlag()
{
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	SetRootComponent(FlagMesh);
	GetAreaSphere()->SetupAttachment(FlagMesh);//将USphereComponent附加到旗子上
	GetPickupWidget()->SetupAttachment(FlagMesh);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::Weapon_Dropped);//先将此件武器的状态的设为丢弃状态
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//这个变量是解绑组件的规则，参数意思是自动计算相对转换，以便分离组件维护相同的世界转换。
	FlagMesh->DetachFromComponent(DetachRules);//不论组件被附加到什么上面都会拆下来，自动解绑被绑在一起的组件
	SetOwner(nullptr);//将拥有者设置为空
	BlasterOwnerCharacter = nullptr;//将拥有此武器的角色类设置为空
	BlasterOwnerController = nullptr;//将拥有此武器的控制器设置为空
}

void AFlag::HandleWeaponEquiped()
{
	ShowPickupWidget(false);//关闭武器的拾取提示
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);//关闭武器的碰撞盒子
	FlagMesh->SetSimulatePhysics(false);//开启模拟物理
	FlagMesh->SetEnableGravity(false);//开启武器重力
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);

}

void AFlag::HandleWeaponDropped()
{
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);//开启武器的碰撞盒子
	}

	FlagMesh->SetSimulatePhysics(true);//开启模拟物理
	FlagMesh->SetEnableGravity(true);//开启武器重力
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block); //设置武器对所有碰撞检测通道的响应模式
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); //覆盖 忽略对Pawn通道的碰撞检测
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); //覆盖 忽略对Camera通道的碰撞检测


}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	//获取角色的初始位置和位移
	InitialTransform = GetActorTransform();
}

void AFlag::ResetFlag()
{
	ABlasterCharacter* FlagBearer = Cast<ABlasterCharacter>(GetOwner());
	if (FlagBearer)
	{
		FlagBearer->SetHoldingTheFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch();
	}

	if (!HasAuthority()) return;

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);//这个变量是解绑组件的规则，参数意思是自动计算相对转换，以便分离组件维护相同的世界转换。
	FlagMesh->DetachFromComponent(DetachRules);//不论组件被附加到什么上面都会拆下来，自动解绑被绑在一起的组件
	SetWeaponState(EWeaponState::Weapon_Initial);//先将此件武器的状态的设为丢弃状态
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
	SetOwner(nullptr);//将拥有者设置为空
	BlasterOwnerCharacter = nullptr;//将拥有此武器的角色类设置为空
	BlasterOwnerController = nullptr;//将拥有此武器的控制器设置为空

	//将旗子送回初始位置
	SetActorTransform(InitialTransform);
}