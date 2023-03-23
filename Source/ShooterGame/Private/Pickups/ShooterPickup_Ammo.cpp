// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Pickups/ShooterPickup_Ammo.h"
#include "Weapons/ShooterWeapon.h"
#include "OnlineSubsystemUtils.h"

AShooterPickup_Ammo::AShooterPickup_Ammo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AmmoClips = 2;
	//Live for 30 seconds
	LiveTime = 30.0;
}

bool AShooterPickup_Ammo::IsForWeapon(UClass* WeaponClass)
{
	return WeaponType->IsChildOf(WeaponClass);
}

bool AShooterPickup_Ammo::CanBePickedUp(AShooterCharacter* TestPawn) const
{
	if(PickupWeapon)
	{
		return true;
	}
	
	AShooterWeapon* TestWeapon = (TestPawn ? TestPawn->FindWeapon(WeaponType) : NULL);
	if (bIsActive && TestWeapon)
	{
		return TestWeapon->GetCurrentAmmo() < TestWeapon->GetMaxAmmo();
	}

	return false;
}

void AShooterPickup_Ammo::GivePickupTo(class AShooterCharacter* Pawn)
{
	AShooterWeapon* Weapon = (Pawn ? Pawn->FindWeapon(WeaponType) : NULL);

	//if doesn't have the weapon
	if(PickupWeapon && !Weapon)
	{	
		//Spawn the weapon
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Instigator = nullptr;
		SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnInfo.OverrideLevel = nullptr;

		UWorld* World = GetWorld();
		
		//No need for location parameter
		AShooterWeapon_Instant* NewWeapon = World->SpawnActor<AShooterWeapon_Instant>(WeaponType.Get(), SpawnInfo);
		NewWeapon->SetWeaponConfig(PickupWeaponConfig);
		
		//Add ammo to the new weapon
		NewWeapon->GiveAmmo(PickupWeaponAmmo);
		//Then give weapon to the pawn
		Pawn->AddWeapon(NewWeapon);
		return;
	}
	
	if (Weapon)
	{
		int32 Qty = AmmoClips * Weapon->GetAmmoPerClip();
		Weapon->GiveAmmo(Qty);

		// Fire event for collected ammo
		if (Pawn)
		{
			const UWorld* World = GetWorld();
			const IOnlineEventsPtr Events = Online::GetEventsInterface(World);
			const IOnlineIdentityPtr Identity = Online::GetIdentityInterface(World);

			if (Events.IsValid() && Identity.IsValid())
			{							
				AShooterPlayerController* PC = Cast<AShooterPlayerController>(Pawn->Controller);
				if (PC)
				{
					ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PC->Player);

					if (LocalPlayer)
					{
						const int32 UserIndex = LocalPlayer->GetControllerId();
						TSharedPtr<const FUniqueNetId> UniqueID = Identity->GetUniquePlayerId(UserIndex);			
						if (UniqueID.IsValid())
						{
							FVector Location = Pawn->GetActorLocation();

							FOnlineEventParms Params;		

							Params.Add( TEXT( "SectionId" ), FVariantData( (int32)0 ) ); // unused
							Params.Add( TEXT( "GameplayModeId" ), FVariantData( (int32)1 ) ); // @todo determine game mode (ffa v tdm)
							Params.Add( TEXT( "DifficultyLevelId" ), FVariantData( (int32)0 ) ); // unused

							Params.Add( TEXT( "ItemId" ), FVariantData( (int32)Weapon->GetAmmoType() + 1 ) ); // @todo come up with a better way to determine item id, currently health is 0 and ammo counts from 1
							Params.Add( TEXT( "AcquisitionMethodId" ), FVariantData( (int32)0 ) ); // unused
							Params.Add( TEXT( "LocationX" ), FVariantData( Location.X ) );
							Params.Add( TEXT( "LocationY" ), FVariantData( Location.Y ) );
							Params.Add( TEXT( "LocationZ" ), FVariantData( Location.Z ) );
							Params.Add( TEXT( "ItemQty" ), FVariantData( (int32)Qty ) );		

							Events->TriggerEvent(*UniqueID, TEXT("CollectPowerup"), Params);
						}
					}
				}
			}
		}
	}
}

void AShooterPickup_Ammo::SetupPickupWeapon() {
	//Now start a timer at the end of which this pickup disappears
	if(PickupWeapon){
		//Set the timer
		GetWorldTimerManager().SetTimer(TimerHandle_Destroy, this, &AShooterPickup_Ammo::DestroyPickup, LiveTime, false);
	}
}

void AShooterPickup_Ammo::DestroyPickup(){
	Destroy();
}

void AShooterPickup_Ammo::OnPickedUp() {
	Super::OnPickedUp();
	DestroyPickup();
}
