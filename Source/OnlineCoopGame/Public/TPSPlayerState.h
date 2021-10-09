//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TPSPlayerState.generated.h"


UCLASS()
class ONLINECOOPGAME_API ATPSPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    UFUNCTION(BlueprintCallable, Category = PlayerState)
    void AddScore(float ScoreDelta);
};
