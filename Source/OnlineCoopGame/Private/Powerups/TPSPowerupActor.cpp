//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "Powerups/TPSPowerupActor.h"
#include "Net/UnrealNetwork.h"

//------------------------------------------------------------------------------------------------------------------------------------------
ATPSPowerupActor::ATPSPowerupActor()
{
    PowerupInterval    = 0.0f;
    TotalNumberOfTicks = 0;

    bisPowerupActive = false;

    SetReplicates(true);
}

//-------------------------------------------------------PUBLIC FUNCTIONS-------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSPowerupActor::ActivatePowerup(AActor* TargetActor)
{
    OnActivated(TargetActor);

    bisPowerupActive = true;
    OnRep_PowerupActive();

    if (PowerupInterval > 0.0f)
    {
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUFunction(this, FName("OnTickPowerup"), TargetActor);

        GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, TimerDelegate, PowerupInterval, true);

        //Or in this way
        //GetWorldTimerManager().SetTimer(TimerHandle_PowerupTick, TimerDelegate.CreateUObject(this, &ATPSPowerupActor::OnTickPowerup, TargetActor), PowerupInterval, true);
    }
    else
    {
        OnTickPowerup(TargetActor);
    }
}

//-------------------------------------------------------PROTECTED FUNCTIONS----------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSPowerupActor::OnTickPowerup(AActor* TargetActor)
{
    TicksProcessed++;

    OnPowerupTicked(TargetActor);

    if (TicksProcessed >= TotalNumberOfTicks)
    {
        OnExpired();

        bisPowerupActive = false;
        OnRep_PowerupActive();

        GetWorldTimerManager().ClearTimer(TimerHandle_PowerupTick);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSPowerupActor::OnRep_PowerupActive()
{
    OnPowerupStateChange(bisPowerupActive);
}


void ATPSPowerupActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATPSPowerupActor, bisPowerupActive);
}