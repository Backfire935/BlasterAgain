
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTERAGAIN_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;//…Ë÷√Õ¨≤Ω
	friend class ABlasterCharacter;

	void EquipWeapon(class AWeapon* WeaponToEquip);

	void DropWeapon();
protected:

	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
		void ServerSetAiming(bool bIsAiming);


private:
	class ABlasterCharacter* Character;

	UPROPERTY(Replicated)
	 AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;


public:	


		
};
