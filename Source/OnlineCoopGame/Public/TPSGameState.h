//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TPSGameState.generated.h"

UENUM(BlueprintType)
enum class EWaveState : uint8
{
    WaitingToStart,

    WaveInProgress,

    //No longer spawning new bots, waiting for players to kill remaining bots
    WaitingToComplete,

    WaveComplete,

    GameOver,
};

UCLASS()
class ONLINECOOPGAME_API ATPSGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    //*******************************************************************************************************************
    //                                          PUBLIC VARIABLES                                                        *
    //*******************************************************************************************************************

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WaveState, Category = GameState)
    EWaveState WaveState;

    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    void SetWaveState(EWaveState NewState);

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    UFUNCTION(BlueprintImplementableEvent, Category = GameState)
    void WaveStateChanged(EWaveState NewState, EWaveState OldState);

    UFUNCTION()
    void OnRep_WaveState(EWaveState OldState);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
