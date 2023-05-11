// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DieSphereBox.generated.h"

UCLASS()
class BLASTERAGAIN_API ADieSphereBox : public AActor
{
	GENERATED_BODY()
	
public:	
	
	ADieSphereBox();
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	class UBoxComponent* HitBoxComp;


public:	
	
	UFUNCTION(BlueprintCallable)//这个地方必须加UFUNCTION子类继承时编辑器才能找到 否则不接受响应
	void BeginHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32  OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable)//这个地方必须加UFUNCTION子类继承时编辑器才能找到 否则不接受响应
	virtual  void OnHitSphere(class ABlasterCharacter* Sphere); //写 class 让编译器去cpp中找AMyPawn
};
