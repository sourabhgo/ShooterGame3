// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterDamageType.generated.h"

// DamageType class that specifies an icon to display
UCLASS(const, Blueprintable, BlueprintType)
class UShooterDamageType : public UDamageType
{
	GENERATED_UCLASS_BODY()

	/** icon displayed in death messages log when killed with this weapon */
	UPROPERTY(EditDefaultsOnly, Category=HUD)
	FCanvasIcon KillIcon;

	/** force feedback effect to play on a player hit by this damage type */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UForceFeedbackEffect *HitForceFeedback;

	/** force feedback effect to play on a player killed by this damage type */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UForceFeedbackEffect *KilledForceFeedback;
};

UCLASS(const, Blueprintable, BlueprintType)
class UFreezeDamageType : public UShooterDamageType
{
	GENERATED_UCLASS_BODY()

};

UCLASS(const, Blueprintable, BlueprintType)
class UShrinkDamageType : public UShooterDamageType
{
	GENERATED_UCLASS_BODY()

	static const int Scale = 10; //shrinks to 1/10

};

UCLASS(const, Blueprintable, BlueprintType)
class UStompDamageType : public UShooterDamageType
{
	GENERATED_UCLASS_BODY()


};