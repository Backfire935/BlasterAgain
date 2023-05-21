#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),//空闲的

	ECS_Reloading UMETA(DisplayName = "Reloading"),//换弹

	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),//换弹
	ECS_SwappingWeapons UMETA(DisplayName = "Swapping Weapons"),
	ECS_Max UMETA(DisplayName = "DefaultMax")

};