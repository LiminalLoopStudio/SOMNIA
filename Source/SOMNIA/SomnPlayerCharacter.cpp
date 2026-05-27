// SOMNIA: VULNERA — SomnPlayerCharacter.cpp
// Implementación de la clase principal del personaje jugable (Niko)

#include "SomnPlayerCharacter.h"

// UE5 — Movement & Camera
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// UE5 — Enhanced Input
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// UE5 — World & Collision
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

// ============================================================
//  CONSTRUCTOR
// ============================================================

ASomnPlayerCharacter::ASomnPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Spring Arm ---
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 6.0f;
    SpringArm->bDoCollisionTest = true;

    // --- Cámara ---
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    // --- Character Movement ---
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
    GetCharacterMovement()->MaxWalkSpeedCrouched = 120.0f;
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->GravityScale = 1.5f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;

    // Auto posesionar al Player 0
    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// ============================================================
//  BEGIN PLAY
// ============================================================

void ASomnPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

// ============================================================
//  TICK
// ============================================================

void ASomnPlayerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateEmotionIra(DeltaTime);
    UpdateJumpTimers(DeltaTime);

    // Interpolación suave de velocidad para Sprint (F0-024)
    const float TargetSpeed = bIsSprinting ? (WalkSpeed * SprintMultiplier) : WalkSpeed;
    GetCharacterMovement()->MaxWalkSpeed = FMath::FInterpTo(
        GetCharacterMovement()->MaxWalkSpeed, TargetSpeed, DeltaTime, 8.0f);
}

// ============================================================
//  SETUP INPUT
// ============================================================

void ASomnPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Move)
            EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ASomnPlayerCharacter::HandleMove);

        if (IA_Look)
            EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ASomnPlayerCharacter::HandleLook);

        if (IA_Jump)
        {
            EIC->BindAction(IA_Jump, ETriggerEvent::Started, this, &ASomnPlayerCharacter::HandleJump);
            EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ASomnPlayerCharacter::HandleStopJump);
        }

        if (IA_Sprint)
        {
            EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this, &ASomnPlayerCharacter::HandleSprintStart);
            EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &ASomnPlayerCharacter::HandleSprintStop);
        }

        if (IA_Crouch)
            EIC->BindAction(IA_Crouch, ETriggerEvent::Started, this, &ASomnPlayerCharacter::HandleCrouchToggle);

        if (IA_Interact)
            EIC->BindAction(IA_Interact, ETriggerEvent::Started, this, &ASomnPlayerCharacter::HandleInteract);

        if (IA_Paraguas)
            EIC->BindAction(IA_Paraguas, ETriggerEvent::Started, this, &ASomnPlayerCharacter::HandleParaguas);

        if (IA_Mascara)
            EIC->BindAction(IA_Mascara, ETriggerEvent::Started, this, &ASomnPlayerCharacter::HandleMascara);
    }
}

// ============================================================
//  INPUT HANDLERS
// ============================================================

void ASomnPlayerCharacter::HandleMove(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller == nullptr) return;

    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation = FRotator(0.0f, Rotation.Yaw, 0.0f);
    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
}

void ASomnPlayerCharacter::HandleLook(const FInputActionValue& Value)
{
    const FVector2D LookVector = Value.Get<FVector2D>();
    AddControllerYawInput(LookVector.X);
    AddControllerPitchInput(LookVector.Y);
}

void ASomnPlayerCharacter::HandleJump()
{
    JumpBufferCounter = JumpBufferTime;
    Jump();
}

void ASomnPlayerCharacter::HandleStopJump()
{
    StopJumping();
}

void ASomnPlayerCharacter::HandleCrouchToggle()
{
    if (bIsCrouched) UnCrouch();
    else Crouch();
}

void ASomnPlayerCharacter::HandleSprintStart() { StartSprint(); }
void ASomnPlayerCharacter::HandleSprintStop() { StopSprint(); }
void ASomnPlayerCharacter::HandleInteract() { Interact(); }
void ASomnPlayerCharacter::HandleParaguas() { OpenCloseParaguas(); }
void ASomnPlayerCharacter::HandleMascara() { ToggleMascara(); }

// ============================================================
//  API PUBLICA — MOVIMIENTO
// ============================================================

void ASomnPlayerCharacter::StartSprint()
{
    if (bIsSprinting) return;
    bIsSprinting = true;
}

void ASomnPlayerCharacter::StopSprint()
{
    if (!bIsSprinting) return;
    bIsSprinting = false;
}

void ASomnPlayerCharacter::SetZoneWalkSpeed(float NewSpeed)
{
    WalkSpeed = NewSpeed;
    GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ?
        WalkSpeed * SprintMultiplier : WalkSpeed;
}

FVector ASomnPlayerCharacter::GetFacingDirection() const
{
    return GetActorForwardVector();
}

// ============================================================
//  API PUBLICA — INTERACCIÓN
// ============================================================

void ASomnPlayerCharacter::Interact()
{
    if (bIsInteracting) return;

    AActor* Target = GetInteractableInFront();
    if (Target == nullptr) return;

    bIsInteracting = true;
    OnInteractionStarted.Broadcast(Target, FName("INTERACT_BASE"));
}

// ============================================================
//  API PUBLICA — ITEMS
// ============================================================

void ASomnPlayerCharacter::OpenCloseParaguas()
{
    if (!bHasParaguas) return;
    bParaguasOpen = !bParaguasOpen;
    OnParaguasStateChanged.Broadcast(bParaguasOpen);
}

void ASomnPlayerCharacter::ToggleMascara()
{
    if (!bHasMascara) return;
    bMascaraEquipped = !bMascaraEquipped;
    OnMascaraStateChanged.Broadcast(bMascaraEquipped);
}

// ============================================================
//  API PUBLICA — EMOTION SYSTEM
// ============================================================

void ASomnPlayerCharacter::AddEmotionIra(float Delta)
{
    const float Anterior = EmotionIra;
    EmotionIra = FMath::Clamp(EmotionIra + Delta, 0.0f, 1.0f);
    if (!FMath::IsNearlyEqual(Anterior, EmotionIra))
        OnEmotionIraChanged.Broadcast(EmotionIra);
}

// ============================================================
//  HELPERS INTERNOS — TICK
// ============================================================

void ASomnPlayerCharacter::UpdateEmotionIra(float DeltaTime)
{
    if (bIsSprinting)
        AddEmotionIra(IraIncreasePerSprintSec * DeltaTime);
    else
        AddEmotionIra(-IraDecayPerIdleSec * DeltaTime);
}

void ASomnPlayerCharacter::UpdateJumpTimers(float DeltaTime)
{
    if (!GetCharacterMovement()->IsMovingOnGround())
        CoyoteTimeCounter -= DeltaTime;
    else
        CoyoteTimeCounter = CoyoteTime;

    if (JumpBufferCounter > 0.0f)
    {
        JumpBufferCounter -= DeltaTime;
        if (GetCharacterMovement()->IsMovingOnGround())
        {
            Jump();
            JumpBufferCounter = 0.0f;
        }
    }
}

AActor* ASomnPlayerCharacter::GetInteractableInFront() const
{
    const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
    const FVector End = Start + GetFacingDirection() * 200.0f;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, Start, End, ECC_Visibility, Params);

#if !UE_BUILD_SHIPPING
    DrawDebugLine(GetWorld(), Start, End,
        bHit ? FColor::Green : FColor::Red, false, 0.1f, 0, 1.5f);
#endif

    return (bHit && HitResult.GetActor()) ? HitResult.GetActor() : nullptr;
}