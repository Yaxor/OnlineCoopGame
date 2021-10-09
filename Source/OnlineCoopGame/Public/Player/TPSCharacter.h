//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "TPSCharacter.generated.h"

class UCameraComponent;
class UTPSHealthComponent;
class USpringArmComponent;
class ASWeapon;

UCLASS()
class ONLINECOOPGAME_API ATPSCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    //!Constructor
    ATPSCharacter();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS & VARIABLES                                        *
    //*******************************************************************************************************************

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* CameraComp;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* SpringArmComp;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
    UTPSHealthComponent* HealthComp;

    UPROPERTY(Replicated)
    ASWeapon* CurrentWeapon;
    UPROPERTY(EditDefaultsOnly, Category = Player)
    TSubclassOf<ASWeapon> DefaultWeapon;

    UPROPERTY(VisibleDefaultsOnly, Category = Player)
    FName WeaponSocket;

    /*Pawn died previpusly*/
    UPROPERTY(ReplicatedUsing = OnRep_Died, BlueprintReadOnly, Category = Player)
    bool bDied;
    bool bWantsToZoom;

    float DefaultFOV;
    UPROPERTY(EditDefaultsOnly, Category = Player)
    float ZoomFOV;
    UPROPERTY(EditDefaultsOnly, Category = Player, meta = (ClampMin = 0.1f, ClampMax = 100.0f))
    float ZoomInterpSpeed;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    virtual void Tick(float DeltaTime) override;

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual FVector GetPawnViewLocation() const override;

    UFUNCTION(BlueprintCallable, Category = Player)
    void StartFireWeapon();
    UFUNCTION(BlueprintCallable, Category = Player)
    void StopFireWeapon();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    virtual void BeginPlay() override;

    void BeginCrouch();
    void BeginZoom();
    void EndCrouch();
    void EndZoom();
    void MoveForward(float Value);
    void MoveRight(float Value);
    void ReloadWeapon();
    
    void ZoomInterp(const float DeltaTime);

    UFUNCTION()
    void OnHealthChanged(UTPSHealthComponent* HealthComponent,
        float Health,
        float HealthDelta,
        const class UDamageType* DamageType,
        class AController* InstigatedBy,
        AActor* DamageCauser);

    UFUNCTION()
    void OnRep_Died();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
