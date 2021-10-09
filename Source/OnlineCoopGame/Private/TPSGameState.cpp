//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "TPSGameState.h"
#include "Net/UnrealNetwork.h"

//-------------------------------------------------------PUBLIC FUNCTIONS-------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameState::SetWaveState(EWaveState NewState)
{
    if (GetLocalRole() == ROLE_Authority)
    {
        EWaveState OldState = WaveState;
        WaveState           = NewState;

        OnRep_WaveState(OldState);
    }
}

//-------------------------------------------------------PROTECTED FUNCTIONS----------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameState::OnRep_WaveState(EWaveState OldState)
{
    WaveStateChanged(WaveState, OldState);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATPSGameState, WaveState);
}
