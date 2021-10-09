//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSPowerupActor.generated.h"

UCLASS()
class ONLINECOOPGAME_API ATPSPowerupActor : public AActor
{
    GENERATED_BODY()

public:
    //!Constructor
    ATPSPowerupActor();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS & VARIABLES                                        *
    //*******************************************************************************************************************

    FTimerHandle TimerHandle_PowerupTick;

    UPROPERTY(ReplicatedUsing = OnRep_PowerupActive)
    bool bisPowerupActive;

    /*Time between powerup ticks*/
    UPROPERTY(EditDefaultsOnly, Category ="Powerups")
    float PowerupInterval;

    //Total number of ticks applied
    int32 TicksProcessed;
    /*Total times we apply the power of the effect*/
    UPROPERTY(EditDefaultsOnly, Category ="Powerups")
    int32 TotalNumberOfTicks;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    void ActivatePowerup(AActor* TargetActor);

    UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
    void OnActivated(AActor* TargetActor);
    UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
    void OnPowerupTicked(AActor* TargetActor);
    UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
    void OnExpired();
    UFUNCTION(BlueprintImplementableEvent, Category = "Powerups")
    void OnPowerupStateChange(bool bNewIsActive);

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    UFUNCTION()
    void OnTickPowerup(AActor* TargetActor);

    UFUNCTION()
    void OnRep_PowerupActive();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
