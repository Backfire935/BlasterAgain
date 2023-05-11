

#include "BulletShells.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


ABulletShells::ABulletShells()
{
 
	PrimaryActorTick.bCanEverTick = true;
	BulletShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellComp"));
	SetRootComponent(BulletShellMesh);

	BulletShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);//设置通道碰撞，对相机忽略
	BulletShellMesh->SetSimulatePhysics(true);//开启模拟物理属性
	BulletShellMesh->SetEnableGravity(true);//开启物理重力
	BulletShellMesh->SetNotifyRigidBodyCollision(true);//模拟生成命中事件
	ShellEjectionImpulse = 10.f;//设置子弹壳抛出的冲量
}


void ABulletShells::BeginPlay()
{
	Super::BeginPlay();
	BulletShellMesh->OnComponentHit.AddDynamic(this, &ABulletShells::OnHit);//将子弹落地的事件添加到委托

	BulletShellMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);//给子弹壳添加一个向前的单位冲量

	SetLifeSpan(5.f);//给这个类设置5s的生命周期
}

void ABulletShells::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (BulletSound)//播放完子弹壳落地的声音后销毁子弹壳
		{
		//BulletShellMesh->SetNotifyRigidBodyCollision(false);//模拟生成命中事件
		//BulletShellMesh->SetSimulatePhysics(true);//关闭模拟物理属性
		UGameplayStatics::PlaySoundAtLocation(this, BulletSound, GetActorLocation());//在子弹壳落地的位置播放一个被命中的音效
		
		}
	BulletShellMesh->SetNotifyRigidBodyCollision(false);
	//Destroy();
}

void ABulletShells::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

