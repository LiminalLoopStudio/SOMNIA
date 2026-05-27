// SOMNIA: VULNERA — SomnPlayerCharacter.h
// Clase principal del personaje jugable (Niko)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SomnPlayerCharacter.generated.h"

// Forward declarations — evitan includes pesados en el .h
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;

// ============================================================
//  DELEGATES — otros sistemas se suscriben a estos eventos
// ============================================================
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmotionIraChanged, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnParaguasStateChanged, bool, bOpen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMascaraStateChanged, bool, bWearing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, FName, ItemID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnteredZone, FName, ZoneID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionStarted, AActor*, Target, FName, InteractionType);

// ============================================================
//  UCLASS
// ============================================================
UCLASS()
class SOMNIA_API ASomnPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:

    ASomnPlayerCharacter();

protected:

    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(
        class UInputComponent* PlayerInputComponent) override;

public:

    virtual void Tick(float DeltaTime) override;

    // ============================================================
    //  COMPONENTES
    // ============================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera",
        meta = (AllowPrivateAccess = "true"))
    UCameraComponent* Camera;

    // ============================================================
    //  INPUT — MAPPING CONTEXT
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* DefaultMappingContext;

    // ============================================================
    //  INPUT — ACTIONS
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Move;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Look;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Jump;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Sprint;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Crouch;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Interact;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Paraguas;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
        meta = (AllowPrivateAccess = "true"))
    UInputAction* IA_Mascara;

    // ============================================================
    //  MOVIMIENTO — PARAMETROS AJUSTABLES
    // ============================================================

    /** Velocidad base de caminar (cm/s). Default 200 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
        meta = (ToolTip = "Velocidad base de caminar en cm/s"))
    float WalkSpeed = 200.0f;

    /** Multiplicador de sprint. SprintSpeed = WalkSpeed * SprintMultiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
        meta = (ToolTip = "Multiplicador de velocidad al correr"))
    float SprintMultiplier = 1.25f;

    /** Velocidad en Zona 2 (Tristeza). Reduccion del 30% */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
        meta = (ToolTip = "Velocidad reducida en Zona 2"))
    float WalkSpeedTristeza = 140.0f;

    /** Tiempo de coyote — permite saltar brevemente tras caer de un borde */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
        meta = (ToolTip = "Ventana de tiempo en segundos para saltar tras caer de un borde"))
    float CoyoteTime = 0.12f;

    /** Buffer de salto — acepta input de salto un poco antes de aterrizar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement",
        meta = (ToolTip = "Ventana de tiempo en segundos para salto anticipado"))
    float JumpBufferTime = 0.08f;

    // ============================================================
    //  EMOTION SYSTEM — PARAMETROS
    // ============================================================

    /** Nivel actual de ira de Niko. Rango 0.0 — 1.0 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion",
        meta = (ClampMin = "0.0", ClampMax = "1.0",
            ToolTip = "Nivel de ira actual. 0 = calmo, 1 = maximo"))
    float EmotionIra = 0.0f;

    /** Cuanto sube EmotionIra por segundo mientras Niko corre */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion",
        meta = (ToolTip = "Incremento de ira por segundo al correr"))
    float IraIncreasePerSprintSec = 0.08f;

    /** Cuanto baja EmotionIra por segundo cuando Niko esta quieto */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion",
        meta = (ToolTip = "Decremento de ira por segundo en reposo"))
    float IraDecayPerIdleSec = 0.04f;

    /** Umbral donde los efectos visuales de alerta se activan */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion",
        meta = (ToolTip = "Umbral de ira para activar efectos de advertencia"))
    float IraWarningThreshold = 0.75f;

    // ============================================================
    //  ESTADO DE ITEMS
    // ============================================================

    UPROPERTY(BlueprintReadOnly, Category = "Items")
    bool bHasParaguas = false;

    UPROPERTY(BlueprintReadOnly, Category = "Items")
    bool bParaguasOpen = false;

    UPROPERTY(BlueprintReadOnly, Category = "Items")
    bool bHasMascara = false;

    UPROPERTY(BlueprintReadOnly, Category = "Items")
    bool bMascaraEquipped = false;

    UPROPERTY(BlueprintReadOnly, Category = "Items")
    bool bHasFrasco = false;

    // ============================================================
    //  API PUBLICA
    // ============================================================

    /** Activa el modo sprint aumentando la velocidad */
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StartSprint();

    /** Desactiva el modo sprint regresando a velocidad normal */
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void StopSprint();

    /** Lanza raycast de interaccion y activa el objeto enfrente */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void Interact();

    /** Abre o cierra el paraguas de Elena */
    UFUNCTION(BlueprintCallable, Category = "Items")
    void OpenCloseParaguas();

    /** Equipa o desequipa la mascara */
    UFUNCTION(BlueprintCallable, Category = "Items")
    void ToggleMascara();

    /** Devuelve el valor actual de EmotionIra (0.0 — 1.0) */
    UFUNCTION(BlueprintPure, Category = "Emotion")
    float GetEmotionIra() const { return EmotionIra; }

    /** Suma Delta a EmotionIra y hace broadcast del delegate */
    UFUNCTION(BlueprintCallable, Category = "Emotion")
    void AddEmotionIra(float Delta);

    /** Devuelve la direccion en que mira Niko (vector normalizado) */
    UFUNCTION(BlueprintPure, Category = "Movement")
    FVector GetFacingDirection() const;

    /** Aplica velocidad de zona — llamado por ZoneManager al entrar a cada zona */
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void SetZoneWalkSpeed(float NewSpeed);

    // ============================================================
    //  DELEGATES PUBLICOS
    // ============================================================

    UPROPERTY(BlueprintAssignable, Category = "Emotion")
    FOnEmotionIraChanged OnEmotionIraChanged;

    UPROPERTY(BlueprintAssignable, Category = "Items")
    FOnParaguasStateChanged OnParaguasStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Items")
    FOnMascaraStateChanged OnMascaraStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Items")
    FOnItemUsed OnItemUsed;

    UPROPERTY(BlueprintAssignable, Category = "Gameplay")
    FOnPlayerEnteredZone OnPlayerEnteredZone;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractionStarted OnInteractionStarted;

protected:

    // ============================================================
    //  ESTADO INTERNO
    // ============================================================

    bool bIsSprinting = false;
    bool bIsInteracting = false;

    float CoyoteTimeCounter = 0.0f;
    float JumpBufferCounter = 0.0f;

    // ============================================================
    //  INPUT HANDLERS
    // ============================================================

    void HandleMove(const FInputActionValue& Value);
    void HandleLook(const FInputActionValue& Value);
    void HandleJump();
    void HandleStopJump();
    void HandleCrouchToggle();
    void HandleSprintStart();
    void HandleSprintStop();
    void HandleInteract();
    void HandleParaguas();
    void HandleMascara();

    // ============================================================
    //  HELPERS INTERNOS
    // ============================================================

    /** Actualiza EmotionIra cada frame segun si Niko corre o descansa */
    void UpdateEmotionIra(float DeltaTime);

    /** Actualiza contadores de CoyoteTime y JumpBuffer */
    void UpdateJumpTimers(float DeltaTime);

    /** Lanza raycast de interaccion hacia adelante (200 cm) */
    AActor* GetInteractableInFront() const;
};