//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+



#include "TPSPlayerState.h"

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSPlayerState::AddScore(float ScoreDelta)
{
    float NewScore = GetScore() + ScoreDelta;
    SetScore(NewScore);
}
