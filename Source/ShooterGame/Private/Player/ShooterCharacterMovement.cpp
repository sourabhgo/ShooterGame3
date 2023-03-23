// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCharacterMovement.h"
#include "Weapons/ShooterWeapon_Instant.h"
#include "Weapons/ShooterDamageType.h"


#pragma region Saved Move

UShooterCharacterMovement::FSavedMove_Shooter::FSavedMove_Shooter()
{
	Saved_bWantsToTeleport = 0;
	Saved_bWantsToJetpack = 0;
	Saved_bWantsToRewind = 0;
	Saved_bWantsToShrink = 0;
	Saved_bWantsToFreeze = 0;
}

bool UShooterCharacterMovement::FSavedMove_Shooter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_Shooter* NewShooterMove = static_cast<FSavedMove_Shooter*>(NewMove.Get());

	if (Saved_bWantsToTeleport != NewShooterMove->Saved_bWantsToTeleport)
	{
		return false;
	}

	if (Saved_bWantsToJetpack != NewShooterMove->Saved_bWantsToJetpack)
	{
		return false;
	}

	if (SavedJetpackFuel != NewShooterMove->SavedJetpackFuel)
	{
		return false;
	}

	if (Saved_bWantsToRewind != NewShooterMove->Saved_bWantsToRewind)
	{
		return false;
	}

	if (Saved_bWantsToShrink != NewShooterMove->Saved_bWantsToShrink)
	{
		return false;
	}

	if (Saved_bWantsToFreeze != NewShooterMove->Saved_bWantsToFreeze)
	{
		return false;
	}

	if (Saved_bWallRunIsRight != NewShooterMove->Saved_bWallRunIsRight)
	{
		return false;
	}

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UShooterCharacterMovement::FSavedMove_Shooter::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToTeleport = 0;
	Saved_bWantsToJetpack = 0;
	Saved_bWantsToRewind = 0;;
	
	SavedMoveDirection = FVector(0);
	SavedJetpackFuel = 0;
	SavedPositions.clear();

	Saved_bWantsToShrink = 0;
	Saved_bWantsToFreeze = 0;

	Saved_bWallRunIsRight = 0;
}

uint8 UShooterCharacterMovement::FSavedMove_Shooter::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();

	if (Saved_bWantsToJetpack) Result |= FLAG_Jetpack;

	return Result;
}

void UShooterCharacterMovement::FSavedMove_Shooter::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UShooterCharacterMovement* CharacterMovement = Cast<UShooterCharacterMovement>(C->GetCharacterMovement());

	Saved_bWantsToTeleport = CharacterMovement->Safe_bWantsToTeleport;
	Saved_bWantsToJetpack = CharacterMovement->Safe_bWantsToJetpack;
	Saved_bWantsToRewind = CharacterMovement->Safe_bWantsToRewind;

	SavedMoveDirection = CharacterMovement->MoveDirection;
	SavedJetpackFuel = CharacterMovement->JetpackFuel;

	//Copy RecordedPositions to SavedPositions
	SavedPositions = CharacterMovement->RecordedPositions;

	Saved_bWantsToShrink = CharacterMovement->Safe_bWantsToShrink;
	Saved_bWantsToFreeze = CharacterMovement->Safe_bWantsToFreeze;

	Saved_bWallRunIsRight = CharacterMovement->Safe_bWallRunIsRight;

}

void UShooterCharacterMovement::FSavedMove_Shooter::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UShooterCharacterMovement* CharacterMovement = Cast<UShooterCharacterMovement>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToTeleport = Saved_bWantsToTeleport;
	CharacterMovement->Safe_bWantsToJetpack = Saved_bWantsToJetpack;
	CharacterMovement->Safe_bWantsToRewind = Saved_bWantsToRewind;

	CharacterMovement->MoveDirection = SavedMoveDirection;
	CharacterMovement->JetpackFuel = SavedJetpackFuel;

	//Copy SavedPositions to RecordedPositions
	CharacterMovement->RecordedPositions = SavedPositions;

	CharacterMovement->Safe_bWantsToShrink = Saved_bWantsToShrink;
	CharacterMovement->Safe_bWantsToFreeze = Saved_bWantsToFreeze;

	CharacterMovement->Safe_bWallRunIsRight = Saved_bWallRunIsRight;

}

#pragma endregion

#pragma region Client Network Prediction Data

UShooterCharacterMovement::FNetworkPredictionData_Client_Shooter::FNetworkPredictionData_Client_Shooter(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UShooterCharacterMovement::FNetworkPredictionData_Client_Shooter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Shooter());
}

#pragma endregion

#pragma region Interface

void UShooterCharacterMovement::TeleportPressed()
{
	Safe_bWantsToTeleport = true;
}

void UShooterCharacterMovement::JetpackPressed()
{
	Safe_bWantsToJetpack = true;
}
void UShooterCharacterMovement::JetpackReleased()
{
	Safe_bWantsToJetpack = false;
}

void UShooterCharacterMovement::RewindPressed()
{
	Safe_bWantsToRewind = true;
}

void UShooterCharacterMovement::ShrinkPressed()
{
	AShooterWeapon_Instant* PlayerWeapon = static_cast<AShooterWeapon_Instant*>(ShooterCharacterOwner->GetWeapon());
	TSubclassOf<UDamageType> WeaponDamageType;
	PlayerWeapon->GetDamageType(WeaponDamageType);

	if (PlayerWeapon->GetWeaponType() == EWeaponType::Gun) {
		bool IsShrinktrue = WeaponDamageType.Get()->IsChildOf(UShrinkDamageType::StaticClass());
		if (!IsShrinktrue)
		{
			//Set gun to shrink
			if (!ShooterCharacterOwner->HasAuthority()) {
				PlayerWeapon->Server_SetDamageType(UShrinkDamageType::StaticClass());
			}
			PlayerWeapon->SetDamageType(UShrinkDamageType::StaticClass());
		}
		else
		{
			//Set gun back to normal
			if (!ShooterCharacterOwner->HasAuthority()) {
				PlayerWeapon->Server_SetDamageType(UShooterDamageType::StaticClass());
			}
			PlayerWeapon->SetDamageType(UShooterDamageType::StaticClass());
		}
	}
}

void UShooterCharacterMovement::FreezePressed()
{
	AShooterWeapon_Instant* PlayerWeapon = static_cast<AShooterWeapon_Instant*>(ShooterCharacterOwner->GetWeapon());
	TSubclassOf<UDamageType> WeaponDamageType;
	PlayerWeapon->GetDamageType(WeaponDamageType);
	bool IsFreezetrue = WeaponDamageType.Get()->IsChildOf(UFreezeDamageType::StaticClass());
	if (!IsFreezetrue)
	{
		//Set gun to shrink
		if (!ShooterCharacterOwner->HasAuthority()) {
			PlayerWeapon->Server_SetDamageType(UFreezeDamageType::StaticClass());
		}
		PlayerWeapon->SetDamageType(UFreezeDamageType::StaticClass());
	}
	else
	{
		//Set gun back to normal
		if (!ShooterCharacterOwner->HasAuthority()) {
			PlayerWeapon->Server_SetDamageType(UShooterDamageType::StaticClass());
		}
		PlayerWeapon->SetDamageType(UShooterDamageType::StaticClass());
	}
}
#pragma endregion

#pragma region CMC
//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Teleport
	Safe_bWantsToTeleport = false;
	TeleportDistance = 1000.0f;

	//Jetpacking
	Safe_bWantsToJetpack = false;
	JetpackForce = 1000.f;
	JetpackAccelerationModifier = 2.5f;
	MaxJetpackFuel = 100.0f;
	JetpackFuel = MaxJetpackFuel;
	JetpackFuelConsumptionRate = 10.0f;
	JetpackFuelRefillRate = 10.0f;

	//Time Rewind
	Safe_bWantsToRewind = false;
	MaxReplayTime = 1.0;
	RewindRecordTime = 3.0;
	RewindRecordInterval = 1 / 30.0;
	RewindActive = false;
	RewindHasStarted = false;
	cacheRewindTime = 0.0;
	cacheTimeDelta = 0.0; //serves as fuel from 0 to RPI
	cacheTimeStart = 0.0;
	cacheTimeRecord = RewindRecordInterval; //As soon as the game starts record first position
	RewindReplayInterval = RewindRecordInterval * (MaxReplayTime / RewindRecordTime);
	inputEnabled = true;
	RewindHUDValue = 0.0;

	//variables for shrink
	Safe_bWantsToShrink = false;
	CacheShrinkTime = 0.0;
	CacheShrinkTimer = 0.0;
	CacheUnShrinkTime = 0.0;
	IsCharShrunk = false;
	HasShrinkStarted = false;
	HasShrinkFinished = false;
	HasShrinkTimerFinished = false;
	HasUnShrinkFinished = false;
	ShrinkHUDValue = ShrinkTime;

	//variables for freeze
	Safe_bWantsToFreeze = false;
	IsFreezeActive = false;
	FreezeTime = 10.0;
	CacheFreezeTimer = 0.0;
	IsCharFrozen = false;
	HasFreezeTimerStarted = false;
	HasFreezeTimerFinished = false;

}

void UShooterCharacterMovement::InitializeComponent()
{
	Super::InitializeComponent();

	ShooterCharacterOwner = Cast<AShooterCharacter>(GetOwner());

	//setting up curve for shrinking
	UShrinkDamageType* ShrinkDamage = NewObject<UShrinkDamageType>(this, TEXT("Shrink Damage"));;
	ShrinkCurveFloat = NewObject<UCurveFloat>(this, TEXT("ShrinkCurveFloat"));
	AddKeyToCurve(ShrinkCurveFloat, 0.0f, 1.0f);
	AddKeyToCurve(ShrinkCurveFloat, InterpTime, 1 / float(ShrinkDamage->Scale));

	//setting up curve for unshrinking
	UnShrinkCurveFloat = NewObject<UCurveFloat>(this, TEXT("UnShrinkCurveFloat"));
	AddKeyToCurve(UnShrinkCurveFloat, 0.0f, 1 / float(ShrinkDamage->Scale));
	AddKeyToCurve(UnShrinkCurveFloat, InterpTime, 1.0f);

	//WallRun
	WallRunGravityScaleCurve = NewObject<UCurveFloat>(this, TEXT("WallRunGravityScaleCurve"));
	AddKeyToCurve(WallRunGravityScaleCurve, -0.5f, 0.4f);
	AddKeyToCurve(WallRunGravityScaleCurve, 0.6, 0);


}

// Network
void UShooterCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//We'll need to set Saved_bWantsToTeleport using Server RPC
	Safe_bWantsToJetpack = (Flags & FSavedMove_Shooter::FLAG_Jetpack) != 0;
	//We'll need to set Saved_bWantsToRewind using Server RPC
	//Shrink and Freeze are set by RPC calls.

}

FNetworkPredictionData_Client* UShooterCharacterMovement::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if (ClientPredictionData == nullptr)
	{
		UShooterCharacterMovement* MutableThis = const_cast<UShooterCharacterMovement*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Shooter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}
	return ClientPredictionData;
}

// Getters / Helpers
float UShooterCharacterMovement::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsTargeting())
		{
			MaxSpeed *= ShooterCharacterOwner->GetTargetingSpeedModifier();
		}
		if (ShooterCharacterOwner->IsRunning())
		{
			MaxSpeed *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
	}

	if (MovementMode != MOVE_Custom) return MaxSpeed;

	switch (CustomMovementMode)
	{
	case CMOVE_WallRun:
		return MaxWallRunSpeed;
	default:
		return MaxSpeed;
	}

	return MaxSpeed;
}

bool UShooterCharacterMovement::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

// Movement Pipeline
void UShooterCharacterMovement::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Teleport
	if (Safe_bWantsToTeleport)
	{
		if (CanTeleport())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Teleport);
			if (!IsServer()) Server_SetTeleport(true);
		}
	}

	// Jetpacking
	if (Safe_bWantsToJetpack)
	{
		if (CanJetpack())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Jetpack);
		}
	}

	// Time Rewind
	if (Safe_bWantsToRewind || RewindActive)
	{
		if (RewindActive)
		{
			//already in rewind no more request
			Safe_bWantsToRewind = false;
		}

		if (CanRewind())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Rewind);
			if (!IsServer()) Server_SetRewind(true);
		}
		else
		{

			Safe_bWantsToRewind = false;

		}
	}

	//Shrink
	if (Safe_bWantsToShrink)
	{	
		if(Is_ShrinkActive())
		{
			//already shrinking no more request
			Safe_bWantsToShrink = false;
		}

		if (CanShrink())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Shrink);
			if (!IsServer()) Server_Set_ShrinkActive(true);
		}
		else
		{
			Safe_bWantsToShrink = false;
		}
	}
	
	//Freeze
	if (Safe_bWantsToFreeze)
	{
		if(Is_FreezeActive())
		{
			//already frizen no more request
			Safe_bWantsToShrink = false;
		}
		
		if (CanFreeze())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Freeze);
			if (!IsServer()) Server_Set_FreezeActive(true);
		}
		else
		{
			Safe_bWantsToFreeze = false;
		}
	}

	//Wall Run
	if (IsFalling())
	{
		TryWallRun();
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UShooterCharacterMovement::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

void UShooterCharacterMovement::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Teleport:
		PhysTeleport(deltaTime, Iterations);
		break;
	case CMOVE_Jetpack:
		PhysJetpack(deltaTime, Iterations);
		break;
	case CMOVE_Rewind:
		PhysRewind(deltaTime, Iterations);
		break;
	case CMOVE_Shrink:
		PhysShrink(deltaTime, Iterations);
		break;
	case CMOVE_Freeze:
		PhysFreeze(deltaTime, Iterations);
		break;
	case CMOVE_WallRun:
		PhysWallRun(deltaTime, Iterations);
		break;
	case CMOVE_WallJump:
		PhysWallJump(deltaTime, Iterations);
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	}
}

void UShooterCharacterMovement::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{

	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

}

void UShooterCharacterMovement::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	//Teleport
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Teleport) ExitTeleport();

	if (IsCustomMovementMode(CMOVE_Teleport)) EnterTeleport(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);

	//Jetpack
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Jetpack) ExitJetpack();

	if (IsCustomMovementMode(CMOVE_Jetpack)) EnterJetpack(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);

	//Time Rewind
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Rewind) ExitRewind();

	if (IsCustomMovementMode(CMOVE_Rewind)) EnterRewind(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);

	//Shrink
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Shrink) ExitShrink();

	if (IsCustomMovementMode(CMOVE_Shrink)) EnterShrink(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);

	//Freeze
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Freeze) ExitFreeze();

	if (IsCustomMovementMode(CMOVE_Freeze)) EnterFreeze(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);

	//Wall Run
	if (IsFalling())
	{
		bOrientRotationToMovement = true;
	}

	if (IsWallRunning() && GetOwnerRole() == ROLE_SimulatedProxy)
	{
		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector End = Start + UpdatedComponent->GetRightVector() * CapR() * 2;
		auto Params = ShooterCharacterOwner->GetIgnoreCharacterParams();
		FHitResult WallHit;
		Safe_bWallRunIsRight = GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
	}
}

void UShooterCharacterMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UShooterCharacterMovement, ScaleValue);
	DOREPLIFETIME(UShooterCharacterMovement, ShrinkHUDValue);
}

#pragma endregion

#pragma region Teleport

void UShooterCharacterMovement::EnterTeleport(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{
}

void UShooterCharacterMovement::ExitTeleport()
{
	SetMovementMode(MOVE_Walking);
}

bool UShooterCharacterMovement::CanTeleport() const
{
	return true;
}

void UShooterCharacterMovement::PhysTeleport(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!IsCustomMovementMode(CMOVE_Teleport))
	{
		SetMovementMode(MOVE_Walking);
		Safe_bWantsToTeleport = false;
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		//Teleport the Actor
		const FVector NewLocation = ShooterCharacterOwner->GetActorLocation() + ShooterCharacterOwner->GetActorForwardVector() * TeleportDistance;
		ShooterCharacterOwner->SetActorLocation(NewLocation, true);
		Safe_bWantsToTeleport = false;

		SetMovementMode(MOVE_Falling);
		StartNewPhysics(remainingTime, Iterations);
		return;
		
	}
}

void UShooterCharacterMovement::Server_SetTeleport_Implementation(bool IsTeleport)
{
	Safe_bWantsToTeleport = IsTeleport;
}

#pragma endregion


#pragma region Jetpack
void UShooterCharacterMovement::EnterJetpack(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{

	if (PawnOwner->IsLocallyControlled())
	{
		MoveDirection = ShooterCharacterOwner->GetLastMovementInputVector();
		if (GetNetMode() == ENetMode::NM_Client)
		{
			ServerSetMoveDirection(MoveDirection);
		}
	}
}

void UShooterCharacterMovement::ExitJetpack()
{
	SetMovementMode(MOVE_Falling);
	Safe_bWantsToJetpack = false;
	//Safe_bWantsToJetpack is set to false upon release
}

bool UShooterCharacterMovement::CanJetpack() const
{
	return JetpackFuel > 0.0f;
}

void UShooterCharacterMovement::PhysJetpack(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!Safe_bWantsToJetpack || !CanJetpack() || !IsCustomMovementMode(CMOVE_Jetpack))
	{
		SetMovementMode(MOVE_Falling);
		Safe_bWantsToJetpack = false;
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		//Jetpack logic
		//burn the fuel
		JetpackFuel = FMath::Clamp(JetpackFuel - JetpackFuelConsumptionRate * timeTick, 0.0f, MaxJetpackFuel);

		FVector OldVelocity = Velocity;

		Velocity.Z += JetpackForce * timeTick;
		Velocity.Y += (MoveDirection.Y * JetpackForce * timeTick) * 0.5f;
		Velocity.X += (MoveDirection.X * JetpackForce * timeTick) * 0.5f;

		const FVector Delta = 0.5f*(OldVelocity + Velocity) * timeTick;
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, ShooterCharacterOwner->GetActorRotation(), true, Hit);

	}
}

float UShooterCharacterMovement::GetMaxAcceleration() const
{
	float NewMaxAcceleration = Super::GetMaxAcceleration();

	if (Safe_bWantsToJetpack)
	{
		NewMaxAcceleration *= JetpackAccelerationModifier;
	}

	return NewMaxAcceleration;
}

void UShooterCharacterMovement::ServerSetMoveDirection_Implementation(FVector NewDir)
{
	MoveDirection = NewDir;
}

#pragma endregion


#pragma region Rewind
void UShooterCharacterMovement::EnterRewind(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{

	//If list is refilled only then check for this
	if (!RewindActive) {
		//Disable Controller Input
		ShooterCharacterOwner->DisableInput(GetWorld()->GetFirstPlayerController());
		inputEnabled = false;

		//I am adding the actor current transform to the tail of linked list to simplify the handling of cases in PhysRewind
		FTransform NewTransform = ShooterCharacterOwner->GetActorTransform();
		RecordedPositions.push_back(NewTransform);
		RewindActive = true; //Now it will be set to false once the rewind is finished
		cacheRewindTime = 0.0;
		RewindHasStarted = false;
	}

	//setting max replay time for HUD
	AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(ShooterCharacterOwner->Controller);
	if (MyPC)
	{
		AShooterPlayerState* MyPlayerState = Cast<AShooterPlayerState>(MyPC->PlayerState);
		if (MyPlayerState)
		{
			MyPlayerState->SetMaxRewindTime(RewindRecordTime);

		}
	}
}

void UShooterCharacterMovement::ExitRewind()
{
}

bool UShooterCharacterMovement::CanRewind() const
{
	int min_size = (RewindRecordTime / RewindRecordInterval);
	if (RecordedPositions.size() < min_size) //list should fill 
	{
		return false;
	}
	return true;
}

void UShooterCharacterMovement::PhysRewind(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!IsCustomMovementMode(CMOVE_Rewind))
	{
		RewindActive = false;
		Safe_bWantsToRewind = false;
		SetMovementMode(MOVE_Walking);
		if (!inputEnabled) {
			ShooterCharacterOwner->EnableInput(GetWorld()->GetFirstPlayerController());
			inputEnabled = true;
		}
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)) && RewindActive)
	{
		Iterations++;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		if (!RewindHasStarted) 
		{
			iter = RecordedPositions.end();
			iter--; //we know size of list is atleast one since we add actor's current position
			//point iter to the last node
			curr = *iter;

		}

			//setup curr and prev
		cacheTimeDelta = cacheRewindTime - cacheTimeStart; //
			
		//handle cases for CTD
		if((cacheTimeDelta >= RewindReplayInterval)||(!RewindHasStarted)) //we can process RPI time worth of time for each iteration
		{
			if (!RewindHasStarted) {
				RewindHasStarted = true;
			}
			//move on to next iteration
			//setup CTS
			cacheTimeStart = cacheRewindTime;
			cacheTimeDelta = 0; //CTD=CRT*CTS

			prev = curr;
			//if list is exhausted or time is up then exit
			if ((iter != RecordedPositions.begin())&&(cacheRewindTime<=MaxReplayTime)){
				iter--;
				curr = *iter;
			}
			else
			{
				//List exhausted
				RewindActive = false;
				Safe_bWantsToRewind = false;
				RecordedPositions.clear();
				SetMovementMode(MOVE_Walking);
				if (!inputEnabled) {
					ShooterCharacterOwner->EnableInput(GetWorld()->GetFirstPlayerController());
					inputEnabled = true;
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
		}

		//We need to process one node each RPI
		//and we spend timeTick for this loop on the machine
		//two cases
		//1. RPI < timeTick
		float remainingTick = timeTick;
		if (RewindReplayInterval < remainingTick) {
			while (remainingTick > RewindReplayInterval)
			{
				remainingTick -= RewindReplayInterval;
				//process one node here
				//here we don't need to do any intepolation
				//we simply move ahead by RPI after changing pairs

				prev = curr;
				//if list is exhausted or time is up then exit
				if ((iter != RecordedPositions.begin()) && (cacheRewindTime <= MaxReplayTime)) {
					iter--;
					curr = *iter;
				}
				else
				{
					//List exhausted
					RewindActive = false;
					Safe_bWantsToRewind = false;
					RecordedPositions.clear();
					SetMovementMode(MOVE_Walking);
					if (!inputEnabled) {
						ShooterCharacterOwner->EnableInput(GetWorld()->GetFirstPlayerController());
						inputEnabled = true;
					}
					StartNewPhysics(remainingTime, Iterations);
					return;
				}

				//Now move the cache variables ahead by RPI
				cacheTimeStart += RewindReplayInterval;
				cacheRewindTime += RewindReplayInterval;
			}
		}

		//2. RPI > timeTick
		//if RPI is greater than remainingTick, then interpolate
		if (RewindReplayInterval > remainingTick)
		{
			if (cacheTimeDelta < RewindReplayInterval)
			{
				//process the pair
				//process for this current iteration-----------------------------
				//Rewind the Actor

				//setup curr pair distance
				float Scalingfactor = cacheTimeDelta / RewindReplayInterval;

				//Get the Delta location
				FVector NewPosition = FMath::Lerp(prev.GetLocation(), curr.GetLocation(), Scalingfactor);

				//Get the delta rotation
				FQuat NewRotation = FMath::Lerp(prev.GetRotation(), curr.GetRotation(), Scalingfactor);
				//this total rotation should happen in replay iterval ie.RPI= RRI*(MRT/RRT)
				//Only timetick/RPI of this is applied.

				FVector NewScale3D = FMath::Lerp(prev.GetScale3D(), curr.GetScale3D(), Scalingfactor);

				FTransform NewTransform(NewRotation, NewPosition, NewScale3D);
				//cannot do this because no '-' operator for FTransform
				//FTransform NewTransform = FMath::Lerp(prev, curr, Scalingfactor);

				// Use interpolation where possible else teleport
				// Get a reference to the navigation system
				const AGameNetworkManager* GameNetworkManager = GetDefault<AGameNetworkManager>();;

				float MaxPositionErrorSquared = GameNetworkManager->MAXPOSITIONERRORSQUARED;

				ETeleportType teleportType;
				if (FMath::Pow((NewPosition - ShooterCharacterOwner->GetActorLocation()).Size(), 2) < MaxPositionErrorSquared)
				{
					//Interpolate
					teleportType = ETeleportType::None;
				}
				else
				{
					//use teleport physics
					teleportType = ETeleportType::TeleportPhysics;
				}
				FHitResult Hit;
				ShooterCharacterOwner->SetActorTransform(NewTransform, false, &Hit, teleportType);

				//Rotate the camera
				if (ShooterCharacterOwner->Controller)
				{
					ShooterCharacterOwner->Controller->SetControlRotation(FRotator(NewRotation));
				}

				cacheTimeDelta += remainingTick; //burn the fuel
				cacheRewindTime += remainingTick; //it shoudl also work on same timeline
			}
		}
		//setting RewindHUDValue
		RewindHUDValue -= timeTick*(RewindRecordTime/MaxReplayTime);
		if (RewindHUDValue < 0)
		{
			RewindHUDValue = 0;
			//ignore the time for extra node added
		}
	}
}

void UShooterCharacterMovement::Server_SetRewind_Implementation(bool IsRewind)
{
	Safe_bWantsToRewind = IsRewind;
}
#pragma endregion

#pragma region Shrink
void UShooterCharacterMovement::EnterShrink(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{
	if (!HasShrinkStarted) {
		//setting max shrink time for HUD
		AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(ShooterCharacterOwner->Controller);
		if (MyPC)
		{
			AShooterPlayerState* MyPlayerState = Cast<AShooterPlayerState>(MyPC->PlayerState);
			if (MyPlayerState)
			{
				MyPlayerState->SetMaxShrinkTime(ShrinkTime);

			}
		}
	}
}

void UShooterCharacterMovement::ExitShrink()
{

}

bool UShooterCharacterMovement::CanShrink() const
{
	return true;
}

void UShooterCharacterMovement::PhysShrink(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!IsCustomMovementMode(CMOVE_Shrink))
	{
		Safe_bWantsToShrink = false;
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		//Start the shrink
		if (!HasShrinkStarted)
		{
			//Store the actor's location
			HasShrinkStarted = true;
		}

		//if currently shrinking
		if (HasShrinkStarted && !HasShrinkFinished)
		{
			//Get back to normal physics and handle shrinking in tick
			Safe_bWantsToShrink = false;
			SetMovementMode(MOVE_Falling);
		}

		//if shrunk, then start the timer
		if (HasShrinkFinished)
		{
			//Get back to normal and handle timer in tick
			Safe_bWantsToShrink = false;
			IsCharShrunk = true;
			SetMovementMode(MOVE_Falling);

		}

		//start unshrinking
		if (HasShrinkTimerFinished && !HasUnShrinkFinished)
		{
			//Get back to normal physics and handle Un-shrinking in tick
			Safe_bWantsToShrink = false;
			IsCharShrunk = false;
			SetMovementMode(MOVE_Falling);

		}

		if (HasUnShrinkFinished)
		{

			//clean up
			IsCharShrunk = false;
			HasShrinkStarted = false;
			HasShrinkFinished = false;
			//HasShrinkTimerStarted = false;
			HasShrinkTimerFinished = false;
			//HasUnShrinkStarted = false;
			HasUnShrinkFinished = false;
			CacheShrinkTime = 0.0;
			CacheShrinkTimer = 0.0;
			CacheUnShrinkTime = 0.0;
			Safe_bWantsToShrink = false;
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}

	}
}

void UShooterCharacterMovement::Set_ShrinkActive(bool isactive) {
	Safe_bWantsToShrink = isactive;
}

void UShooterCharacterMovement::Server_Set_ShrinkActive_Implementation(bool isactive) {
	Safe_bWantsToShrink = isactive;
}

UFUNCTION()
void UShooterCharacterMovement::OnRep_ScaleValue()
{

	// Handle changes to ScaleValue on the simulated proxy
	//scale the character
	ShooterCharacterOwner->SetActorScale3D(FVector(ScaleValue));
}

UFUNCTION()
void UShooterCharacterMovement::OnRep_ShrinkHUDValue()
{
	//update the fuel for HUD
	AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(ShooterCharacterOwner->Controller);
	if (MyPC)
	{
		AShooterPlayerState* MyPlayerState = Cast<AShooterPlayerState>(MyPC->PlayerState);
		if (MyPlayerState)
		{
			//Use client's shrinktime
			MyPlayerState->SetMaxShrinkTime(ShrinkTime);

			MyPlayerState->SetShrinkTime(ShrinkHUDValue);
		}
	}
}
#pragma endregion

#pragma region Freeze

void UShooterCharacterMovement::EnterFreeze(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{

}

void UShooterCharacterMovement::ExitFreeze()
{

}

bool UShooterCharacterMovement::CanFreeze() const
{
	return true;
}

void UShooterCharacterMovement::PhysFreeze(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!IsCustomMovementMode(CMOVE_Freeze))
	{
		Safe_bWantsToFreeze = false;
		IsFreezeActive = false;
		//enable movement
		if (!inputEnabled) {
			//enable movement
			//enable movement
			FreezePlayer(false);
			if (!IsServer())
			{
				Server_FreezePlayer(false);
			}
			inputEnabled = true;
		}
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		//Freezing the character
		if (!IsCharFrozen) {
			//Because it's a simulated proxy it wouldn't have a controller and the above line wuld fail.
			//We need to run this code on server
			FreezePlayer(true);
			if (!IsServer())
			{
				Server_FreezePlayer(true);
			}
			inputEnabled = false;

			IsCharFrozen = true;
			IsFreezeActive = true;
		}

		//Start running Timer
		if (!HasFreezeTimerStarted)
		{
			HasFreezeTimerStarted = true;
		}

		//If in the middle of Timer
		if (HasFreezeTimerFinished)
		{
			//enable movement
			FreezePlayer(false);
			if (!IsServer())
			{
				Server_FreezePlayer(false);
			}
			inputEnabled = true;

			//cleanup
			IsCharFrozen = false;
			Safe_bWantsToFreeze = false;
			IsFreezeActive = false;
			CacheFreezeTimer = 0.0;


			//Timer has finished
			HasFreezeTimerFinished = false;
			HasFreezeTimerStarted = false;
			SetMovementMode(MOVE_Walking); //enable movement
			StartNewPhysics(deltaTime, Iterations);
			return;
		}
	}
}

bool UShooterCharacterMovement::Is_FreezeActive() {
	return IsFreezeActive;
}

bool UShooterCharacterMovement::Is_ShrinkActive() {
	return (HasShrinkStarted && !HasUnShrinkFinished);
}

void UShooterCharacterMovement::Set_FreezeActive(bool isactive) {
	Safe_bWantsToFreeze = isactive;
}

void UShooterCharacterMovement::Server_Set_FreezeActive_Implementation(bool isactive) {
	Safe_bWantsToFreeze = isactive;
}

void UShooterCharacterMovement::FreezePlayer(bool value) {
	if (ShooterCharacterOwner->Controller)
	{
		ShooterCharacterOwner->Controller->SetIgnoreLookInput(value);
		ShooterCharacterOwner->Controller->SetIgnoreMoveInput(value);
	}
}

void UShooterCharacterMovement::Server_FreezePlayer_Implementation(bool value) {
	if (ShooterCharacterOwner->Controller)
	{
		ShooterCharacterOwner->Controller->SetIgnoreLookInput(value);
		ShooterCharacterOwner->Controller->SetIgnoreMoveInput(value);
	}
}

#pragma endregion

#pragma region Wall Run
bool UShooterCharacterMovement::CanAttemptJump() const
{
	return Super::CanAttemptJump() || IsWallRunning() || IsFalling();
}

bool UShooterCharacterMovement::DoJump(bool bReplayingMoves)
{

	bool bWasWallRunning = IsWallRunning();
	if (Super::DoJump(bReplayingMoves))
	{
		if (bWasWallRunning)
		{
			FVector Start = UpdatedComponent->GetComponentLocation();
			FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
			FVector End = Safe_bWallRunIsRight ? Start + CastDelta : Start - CastDelta;
			auto Params = ShooterCharacterOwner->GetIgnoreCharacterParams();
			FHitResult WallHit;
			GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
			Velocity += WallHit.Normal * WallJumpOffForce;
		}


		return true;
	}
	return false;
}

bool UShooterCharacterMovement::TryWallRun()
{
	if (!IsFalling()) return false;
	if (Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2)) return false;
	if (Velocity.Z < -MaxVerticalWallRunSpeed) return false;
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector LeftEnd = Start - UpdatedComponent->GetRightVector() * CapR() * 2;
	FVector RightEnd = Start + UpdatedComponent->GetRightVector() * CapR() * 2;
	auto Params = ShooterCharacterOwner->GetIgnoreCharacterParams();
	FHitResult FloorHit, WallHit;
	// Check Player Height
	if (GetWorld()->LineTraceSingleByProfile(FloorHit, Start, Start + FVector::DownVector * (CapHH() + MinWallRunHeight), "BlockAll", Params))
	{
		return false;
	}

	// Left Cast
	GetWorld()->LineTraceSingleByProfile(WallHit, Start, LeftEnd, "BlockAll", Params);
	if (WallHit.IsValidBlockingHit() && (Velocity | WallHit.Normal) < 0)
	{
		Safe_bWallRunIsRight = false;
	}
	// Right Cast
	else
	{
		GetWorld()->LineTraceSingleByProfile(WallHit, Start, RightEnd, "BlockAll", Params);
		if (WallHit.IsValidBlockingHit() && (Velocity | WallHit.Normal) < 0)
		{
			Safe_bWallRunIsRight = true;
		}
		else
		{
			return false;
		}
	}
	FVector ProjectedVelocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);
	if (ProjectedVelocity.SizeSquared2D() < pow(MinWallRunSpeed, 2)) return false;

	// Passed all conditions
	Velocity = ProjectedVelocity;
	Velocity.Z = FMath::Clamp(Velocity.Z, 0.f, MaxVerticalWallRunSpeed);
	SetMovementMode(MOVE_Custom, CMOVE_WallRun);
	return true;
}

void UShooterCharacterMovement::PhysWallRun(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	bJustTeleported = false;
	float remainingTime = deltaTime;
	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();

		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
		FVector End = Safe_bWallRunIsRight ? Start + CastDelta : Start - CastDelta;
		FCollisionQueryParams Params = ShooterCharacterOwner->GetIgnoreCharacterParams();
		float SinPullAwayAngle = FMath::Sin(FMath::DegreesToRadians(WallRunPullAwayAngle));
		FHitResult WallHit;
		GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
		bool bWantsToPullAway = WallHit.IsValidBlockingHit() && !Acceleration.IsNearlyZero() && (Acceleration.GetSafeNormal() | WallHit.Normal) > SinPullAwayAngle;
		if (!WallHit.IsValidBlockingHit() || bWantsToPullAway)
		{
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}
		// Clamp Acceleration
		Acceleration = FVector::VectorPlaneProject(Acceleration, WallHit.Normal);
		Acceleration.Z = 0.f;
		// Apply acceleration
		CalcVelocity(timeTick, 0.f, false, GetMaxBrakingDeceleration());
		Velocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);
		float TangentAccel = Acceleration.GetSafeNormal() | Velocity.GetSafeNormal2D();
		bool bVelUp = Velocity.Z > 0.f;
		Velocity.Z += GetGravityZ() * WallRunGravityScaleCurve->GetFloatValue(bVelUp ? 0.f : TangentAccel) * timeTick;
		if (Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2) || Velocity.Z < -MaxVerticalWallRunSpeed)
		{
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}

		// Compute move parameters
		const FVector Delta = timeTick * Velocity; // dx = v * dt
		const bool bZeroDelta = Delta.IsNearlyZero();
		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			FHitResult Hit;
			SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
			FVector WallAttractionDelta = -WallHit.Normal * WallAttractionForce * timeTick;
			SafeMoveUpdatedComponent(WallAttractionDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
		}
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick; // v = dx / dt
	}


	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
	FVector End = Safe_bWallRunIsRight ? Start + CastDelta : Start - CastDelta;
	auto Params = ShooterCharacterOwner->GetIgnoreCharacterParams();
	FHitResult FloorHit, WallHit;
	GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
	GetWorld()->LineTraceSingleByProfile(FloorHit, Start, Start + FVector::DownVector * (CapHH() + MinWallRunHeight * .5f), "BlockAll", Params);
	if (FloorHit.IsValidBlockingHit() || !WallHit.IsValidBlockingHit() || Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2))
	{
		SetMovementMode(MOVE_Falling);
	}
}
#pragma endregion

#pragma region WallJump
void UShooterCharacterMovement::PhysWallJump(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!IsCustomMovementMode(CMOVE_WallJump))
	{
		SetMovementMode(MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
		FVector End = Start + UpdatedComponent->GetRightVector() * CapR() * 2;
		auto Params = ShooterCharacterOwner->GetIgnoreCharacterParams();
		FHitResult WallHit;
		bool Safe_bWallJumpIsRight = GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
		if (Safe_bWallJumpIsRight) {
			End = Start + CastDelta;
		}
		else
		{
			End = Start - CastDelta;
			bool Safe_bWallJumpIsLeft = GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
		}

		if (WallHit.IsValidBlockingHit()) {
			Velocity.Z += LateralJumpStrength;
			//Use more force to perform lateral jump
			Velocity += WallHit.Normal * (LateralJumpStrength);
		}


		//should fall over all the time
		SetMovementMode(MOVE_Falling);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}
}

void  UShooterCharacterMovement::Server_SetWallJump_Implementation() {
	SetMovementMode(MOVE_Custom, CMOVE_WallJump);
}

#pragma endregion

#pragma region Helpers
bool UShooterCharacterMovement::IsServer() const
{
	return ShooterCharacterOwner->HasAuthority();
}

void UShooterCharacterMovement::AddKeyToCurve(UCurveFloat* Curve, float Time, float Value)
{
	FRichCurve& FloatCurve = Curve->FloatCurve;

	FRichCurveKey Key;
	Key.Time = Time;
	Key.Value = Value;
	FloatCurve.Keys.Add(Key);
}

float UShooterCharacterMovement::CapR() const
{
	return ShooterCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UShooterCharacterMovement::CapHH() const
{
	return ShooterCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

#pragma endregion

#pragma region Tick
void UShooterCharacterMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) {

	//for time rewind
	if (!RewindActive) {
		if (cacheTimeRecord < RewindRecordInterval) {
			cacheTimeRecord += DeltaTime;
		}
		else
		{
			cacheTimeRecord -= RewindRecordInterval;
			if (RecordedPositions.size() >= (RewindRecordTime / RewindRecordInterval))
			{
				RecordedPositions.pop_front();
			}
			else
			{
				RewindHUDValue += RewindRecordInterval;
			}
			FTransform NewTransform = ShooterCharacterOwner->GetActorTransform();
			//RecordedPositions.push_back(PosRot(NewTransform.GetLocation(), NewTransform.GetRotation()));
			RecordedPositions.push_back(NewTransform);
		}
	}

	//if currently shrinking
	if (HasShrinkStarted && !HasShrinkFinished)
	{
		if (CacheShrinkTime < InterpTime)
		{
			//Get the value from curve and Scale the character
			CacheShrinkTime += DeltaTime;
			ScaleValue = ShrinkCurveFloat->GetFloatValue(CacheShrinkTime);

			//scale the character
			ShooterCharacterOwner->SetActorScale3D(FVector(ScaleValue));
		}
		else
		{
			HasShrinkFinished = true;
			IsCharShrunk = true;
		}
	}

	//for shrink
	//Run the timer
	if (!HasShrinkTimerFinished && IsCharShrunk)
	{
		if (CacheShrinkTimer < ShrinkTime)
		{
			//Timer is running
			CacheShrinkTimer += DeltaTime;
			//set value for HUD
			ShrinkHUDValue = CacheShrinkTimer;
		}
		else
		{
			HasShrinkTimerFinished = true;
			IsCharShrunk = false;
			SetMovementMode(MOVE_Custom, CMOVE_Shrink);
		}
	}

	//start unshrinking
	if (HasShrinkTimerFinished && !HasUnShrinkFinished)
	{
		if (CacheUnShrinkTime < InterpTime)
		{
			//Get the value from curve and Scale the character
			CacheUnShrinkTime += DeltaTime;
			ScaleValue = UnShrinkCurveFloat->GetFloatValue(CacheUnShrinkTime);

			//scale the character
			ShooterCharacterOwner->SetActorScale3D(FVector(ScaleValue));

		}
		else
		{
			//Done!
			HasUnShrinkFinished = true; //would be eventually set to false
			SetMovementMode(MOVE_Custom, CMOVE_Shrink);
			//Finish
		}

	}

	//for freeze
	//Run the timer
	if (HasFreezeTimerStarted && !HasFreezeTimerFinished)
	{
		if (CacheFreezeTimer < FreezeTime)
		{
			//Timer is running
			CacheFreezeTimer += DeltaTime;
		}
		else
		{
			HasFreezeTimerFinished = true;
			SetMovementMode(MOVE_Custom, CMOVE_Freeze);
		}
	}
	
	//if not jetpacking then recharge fuel
	if(!IsCustomMovementMode(CMOVE_Jetpack))
	{
		//refill jetpack
		if (JetpackFuel < MaxJetpackFuel) JetpackFuel = FMath::Clamp(JetpackFuel + JetpackFuelRefillRate * DeltaTime, 0.0f, MaxJetpackFuel);
	}

	//update the fuel for HUD
	AShooterPlayerController* MyPC = Cast<AShooterPlayerController>(ShooterCharacterOwner->Controller);
	if (MyPC)
	{
		AShooterPlayerState* MyPlayerState = Cast<AShooterPlayerState>(MyPC->PlayerState);
		if (MyPlayerState)
		{
			MyPlayerState->SetJetpackFuel(JetpackFuel);
			MyPlayerState->SetRewindTime(RewindHUDValue);
			MyPlayerState->SetShrinkTime(ShrinkHUDValue);
		}
	}

	//WallJump
	if (IsFalling() && ShooterCharacterOwner->bPressedJump)
	{
		SetMovementMode(MOVE_Custom, CMOVE_WallJump);
		if (!IsServer()) Server_SetWallJump();
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
#pragma endregion

