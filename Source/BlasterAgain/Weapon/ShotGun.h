#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotGun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTERAGAIN_API AShotGun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);//获取喷子子弹群的落点

	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
private:
	UPROPERTY(EditAnywhere,Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10;//喷子弹丸数量
};
