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

    // --- Spring Arm (brazo del boom de cámara) ---
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 400.0f;   // distancia base de cámara
    SpringArm->bUsePawnControlRotation = true;     // el brazo rota con el controller
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 6.0f;     // SmoothSpeedNormal del TDD
    SpringArm->bDoCollisionTest = true;     // collision avoidance básico integrado

    // --- Cámara ---
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;        // la cámara no rota sola

    // --- Character Movement — valores base del TDD ---
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;       // 200 cm/s
    GetCharacterMovement()->MaxWalkSpeedCrouched = 120.0f;
    GetCharacterMovement()->JumpZVelocity = 420.0f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->GravityScale = 1.5f;
    GetCharacterMovement()->bOrientRotationToMovement = true;

    // Niko no rota con el controller — rota hacia donde se mueve
    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

// ============================================================
//  BEGIN PLAY
// ============================================================

void ASomnPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Agregar el Input Mapping Context al Enhanced Input Subsystem
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

    // Velocidad inicial correcta
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
}

// ============================================================
//  SETUP INPUT
// ============================================================

void ASomnPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Movimiento — triggered cada frame mientras se presiona
        if (IA_Move)
        {
            EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this,
                &ASomnPlayerCharacter::HandleMove);
        }

        // Cámara — triggered cada frame mientras se mueve el ratón/stick
        if (IA_Look)
        {
            EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this,
                &ASomnPlayerCharacter::HandleLook);
        }

        // Salto — Started al presionar, Completed al soltar
        if (IA_Jump)
        {
            EIC->BindAction(IA_Jump, ETriggerEvent::Started, this,
                &ASomnPlayerCharacter::HandleJump);
            EIC->BindAction(IA_Jump, ETriggerEvent::Completed, this,
                &ASomnPlayerCharacter::HandleStopJump);
        }

        // Sprint — Started al presionar, Completed al soltar
        if (IA_Sprint)
        {
            EIC->BindAction(IA_Sprint, ETriggerEvent::Started, this,
                &ASomnPlayerCharacter::HandleSprintStart);
            EIC->BindAction(IA_Sprint, ETriggerEvent::Completed, this,
                &ASomnPlayerCharacter::HandleSprintStop);
        }

        // Crouch — toggle al presionar
        if (IA_Crouch)
        {
            EIC->BindAction(IA_Crouch, ETriggerEvent::Started, this,
                &ASomnPlayerCharacter::HandleCrouchToggle);
        }

        // Interacción — Started al presionar
        if (IA_Interact)
        {
            EIC->BindAction(IA_Interact, ETriggerEvent::Started, this,
                &ASomnPlayerCharacter::HandleInteract);
        }

        // Paraguas — Started al presionar
        if (IA_Paraguas)
        {
            EIC->BindAction(IA_Paraguas, ETriggerEvent::Started, this,
                &ASomnPlayerCharacter::HandleParaguas);
        }

        // Máscara — Started al presionar
        if (IA_Mascara)
        {
            EIC->BindAction(IA_Mascara, ETriggerEvent::Started, this,
                &ASomnPlayerCharacter::HandleMascara);
        }
    }
}

// ============================================================
//  INPUT HANDLERS
// ============================================================

void ASomnPlayerCharacter::HandleMove(const FInputActionValue& Value)
{
    // Value es un Vector2D: X = derecha/izquierda, Y = adelante/atrás
    const FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller == nullptr) return;

    // Obtener la rotación del controller (solo Yaw, sin Pitch ni Roll)
    const FRotator Rotation = Controller->GetControlRotation();
    const FRotator YawRotation = FRotator(0.0f, Rotation.Yaw, 0.0f);

    // Calcular dirección forward y right respecto al controller
    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // Aplicar movimiento
    AddMovementInput(ForwardDirection, MovementVector.Y);
    AddMovementInput(RightDirection, MovementVector.X);
}

void ASomnPlayerCharacter::HandleLook(const FInputActionValue& Value)
{
    // Value es un Vector2D: X = yaw (horizontal), Y = pitch (vertical)
    const FVector2D LookVector = Value.Get<FVector2D>();

    AddControllerYawInput(LookVector.X);
    AddControllerPitchInput(LookVector.Y);
}

void ASomnPlayerCharacter::HandleJump()
{
    // Salto con soporte de JumpBuffer:
    // Si Niko está en el aire pero hay buffer activo, intentar saltar igual
    JumpBufferCounter = JumpBufferTime;
    Jump();
}

void ASomnPlayerCharacter::HandleStopJump()
{
    StopJumping();
}

void ASomnPlayerCharacter::HandleCrouchToggle()
{
    if (bIsCrouched)
    {
        UnCrouch();
    }
    else
    {
        Crouch();
    }
}

void ASomnPlayerCharacter::HandleSprintStart()
{
    StartSprint();
}

void ASomnPlayerCharacter::HandleSprintStop()
{
    StopSprint();
}

void ASomnPlayerCharacter::HandleInteract()
{
    Interact();
}

void ASomnPlayerCharacter::HandleParaguas()
{
    OpenCloseParaguas();
}

void ASomnPlayerCharacter::HandleMascara()
{
    ToggleMascara();
}

// ============================================================
//  API PUBLICA — MOVIMIENTO
// ============================================================

void ASomnPlayerCharacter::StartSprint()
{
    if (bIsSprinting) return;

    bIsSprinting = true;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SprintMultiplier;

    // EmotionIra sube mientras Niko corre — gestionado en UpdateEmotionIra()
}

void ASomnPlayerCharacter::StopSprint()
{
    if (!bIsSprinting) return;

    bIsSprinting = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ASomnPlayerCharacter::SetZoneWalkSpeed(float NewSpeed)
{
    // Llamado por ZoneManager al entrar a una zona.
    // Ejemplo: ZoneManager llama SetZoneWalkSpeed(140.0f) al entrar a Zona 2.
    WalkSpeed = NewSpeed;

    // Si está sprintando, ajustar también el sprint
    if (bIsSprinting)
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed * SprintMultiplier;
    }
    else
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    }
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

    // Broadcast para que PuzzleManager, InventorySystem, etc. reaccionen
    OnInteractionStarted.Broadcast(Target, FName("INTERACT_BASE"));

    // bIsInteracting se resetea desde Blueprint o desde el sistema receptor
    // Esto permite que la animación de interacción controle el timing
}

// ============================================================
//  API PUBLICA — ITEMS
// ============================================================

void ASomnPlayerCharacter::OpenCloseParaguas()
{
    // Solo funciona si Niko tiene el paraguas
    if (!bHasParaguas) return;

    bParaguasOpen = !bParaguasOpen;

    // Broadcast — ParaguasSystem y ZoneManager reaccionan a este evento
    OnParaguasStateChanged.Broadcast(bParaguasOpen);
}

void ASomnPlayerCharacter::ToggleMascara()
{
    // Solo funciona si Niko tiene la máscara ensamblada
    if (!bHasMascara) return;

    bMascaraEquipped = !bMascaraEquipped;

    // Broadcast — MascaraSystem y AIManager (marionetas) reaccionan
    OnMascaraStateChanged.Broadcast(bMascaraEquipped);
}

// ============================================================
//  API PUBLICA — EMOTION SYSTEM
// ============================================================

void ASomnPlayerCharacter::AddEmotionIra(float Delta)
{
    const float Anterior = EmotionIra;

    // Clamp estricto 0.0 — 1.0 (requerimiento del TDD)
    EmotionIra = FMath::Clamp(EmotionIra + Delta, 0.0f, 1.0f);

    // Solo broadcast si el valor cambió
    if (!FMath::IsNearlyEqual(Anterior, EmotionIra))
    {
        OnEmotionIraChanged.Broadcast(EmotionIra);
    }
}

// ============================================================
//  HELPERS INTERNOS — TICK
// ============================================================

void ASomnPlayerCharacter::UpdateEmotionIra(float DeltaTime)
{
    // EmotionIra solo es relevante en Zona 3 (Ira).
    // El ZoneManager activa/desactiva este sistema vía Blueprint.
    // Aquí hacemos el cálculo frame a frame.

    if (bIsSprinting)
    {
        // Correr sube la ira
        AddEmotionIra(IraIncreasePerSprintSec * DeltaTime);
    }
    else
    {
        // Quietud/caminar la baja
        AddEmotionIra(-IraDecayPerIdleSec * DeltaTime);
    }
}

void ASomnPlayerCharacter::UpdateJumpTimers(float DeltaTime)
{
    // Coyote Time: ventana para saltar después de caer de un borde
    if (!GetCharacterMovement()->IsMovingOnGround())
    {
        CoyoteTimeCounter -= DeltaTime;
    }
    else
    {
        CoyoteTimeCounter = CoyoteTime; // resetear mientras está en el suelo
    }

    // Jump Buffer: aceptar input de salto un poco antes de aterrizar
    if (JumpBufferCounter > 0.0f)
    {
        JumpBufferCounter -= DeltaTime;

        // Si aterrizó mientras había buffer activo, saltar
        if (GetCharacterMovement()->IsMovingOnGround())
        {
            Jump();
            JumpBufferCounter = 0.0f;
        }
    }
}

AActor* ASomnPlayerCharacter::GetInteractableInFront() const
{
    // Raycast de 200 cm hacia adelante desde los ojos de Niko
    const FVector Start = GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
    const FVector End = Start + GetFacingDirection() * 200.0f;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    // Solo colisiona con el canal de visibilidad (objetos del mundo)
    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECC_Visibility,
        Params
    );

    // Línea de debug — visible solo en Development builds
#if !UE_BUILD_SHIPPING
    DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red,
        false, 0.1f, 0, 1.5f);
#endif

    if (bHit && HitResult.GetActor())
    {
        return HitResult.GetActor();
    }

    return nullptr;
}