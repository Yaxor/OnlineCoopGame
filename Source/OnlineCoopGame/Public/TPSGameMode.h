//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TPSGameMode.generated.h"

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, DeadActor, AActor*, KillerActor, AController*, KillerController);

UCLASS()
class ONLINECOOPGAME_API ATPSGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    //!Constructor
    ATPSGameMode();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED VARIABLES                                                     *
    //*******************************************************************************************************************

    FTimerHandle TimerHandle_BotSpawner;
    FTimerHandle TimerHandle_NextWaveStart;

    UPROPERTY(EditDefaultsOnly, Category = GameMode)
    float TimeBetweenWaves;

    //Bots to spawn in current wave
    int32 NrOfBotsToSpawn;
    int32 WaveCount;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    virtual void StartPlay() override;

    UPROPERTY(BlueprintAssignable, Category = GameMode)
    FOnActorKilled OnActorKilled;

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    //Hook for BP to spawn a single bot
    UFUNCTION(BlueprintImplementableEvent, Category = GameMode)
    void SpawnNewBot();

    virtual void Tick(float DeltaSeconds) override;

    //Check if any enemies are still alive
    void CheckWaveState();
    //Check if any player are still alive
    void CheckAnyPlayerAlive();
    //Stop spawning bots
    void EndWave();
    void GameOver();
    //Set timer for next starwave
    void PrepareForNextWave();
    //Respawn dead players
    void RestartDeadPlayers();
    void SetWaveState(EWaveState NewState);
    //Spawn bots and check the number to end wave
    void SpawnBotTimerElapsed();
    //Start timer to spawning bots
    void StartWave();
};
