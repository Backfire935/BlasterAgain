

#include "DieSphereBox.h"

#include "BlasterAgain/Character/BlasterCharacter.h"
#include "BlasterAgain/PlayerController/BlasterPlayerController.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"


ADieSphereBox::ADieSphereBox()
{
	PrimaryActorTick.bCanEverTick = true;
	HitBoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBoxComp"));

	//StaticMesh->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh1"));
	//StaticMesh->SetupAttachment(HitBoxComp);
	HitBoxComp->OnComponentBeginOverlap.AddDynamic(this, &ADieSphereBox::BeginHit); //绑定HitBox组件到BeginHit函数 进去搜宏找参数列表
}

void ADieSphereBox::BeginHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Cast<ABlasterCharacter>(OtherActor))//如果OtherActor转换ASphereBase成功的话 就执行下面的
		{
		ABlasterCharacter* Sphere = Cast<ABlasterCharacter>(OtherActor);
		OnHitSphere(Sphere);
		}

}

void ADieSphereBox::OnHitSphere(ABlasterCharacter* Sphere)
{
	ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(Sphere->Controller);
	//自己对自己造成10000伤害
	UGameplayStatics::ApplyDamage(
		Sphere,
		10000.f,//造成一万点伤害
		OwnerController,
		this,
		UDamageType::StaticClass()
	);
}