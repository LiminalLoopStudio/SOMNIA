// SOMNIA: VULNERA — SomnPlayerCharacter.h
// Clase principal del personaje jugable (Niko)

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SomnPlayerCharacter.generated.h"

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

	// --- INPUT ACTIONS ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
		meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
		meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input",
		meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	// --- FUNCIONES DE MOVIMIENTO ---
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

public:
	virtual void Tick(float DeltaTime) override;

	// --- PARAMETROS TUNEABLES ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchWalkSpeed = 120.0f;
};