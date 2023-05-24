
#include "FPSAimCamera.h"

#include "Camera/CameraComponent.h"


AFPSAimCamera::AFPSAimCamera()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	AimCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("AimCamera"));
	
}

void AFPSAimCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFPSAimCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

