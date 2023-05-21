// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"
UProjectileMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(
	const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);

	return EHandleBlockingHitResult::AdvanceNextSubstep;//直接跳过这次碰撞
	//进入下一个模拟更新。通常在可以忽略碰撞或多次回弹的逻辑时使用，比如当一个挡住了投射物的物体被摧毁时，运动应该继续
}

void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	//火箭不应该停下来，只有在碰撞盒子检测到了撞击才会爆炸
	
}
