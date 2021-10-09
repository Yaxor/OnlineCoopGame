//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "TPSGameMode.h"
#include "TPSGameState.h"
#include "TPSPlayerState.h"

#include "MyComponents/TPSHealthComponent.h"

//------------------------------------------------------------------------------------------------------------------------------------------
ATPSGameMode::ATPSGameMode()
{
    TimeBetweenWaves = 2.0f;

    GameStateClass   = ATPSGameState::StaticClass();
    PlayerStateClass = ATPSPlayerState::StaticClass();

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    CheckWaveState();
    CheckAnyPlayerAlive();
}

//-------------------------------------------------------PUBLIC FUNCTIONS-------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::StartPlay()
{
    Super::StartPlay();

    PrepareForNextWave();
}

//-------------------------------------------------------PROTECTED FUNCTIONS----------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::CheckWaveState()
{
    bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

    if (NrOfBotsToSpawn > 0 || bIsPreparingForWave)
    {
        return;
    }

    bool bIsAnyBotAlive = false;

    for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
    {
        APawn* TestPawn = It->Get();
        if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
        {
            continue;
        }

        UTPSHealthComponent* HealthComp = Cast<UTPSHealthComponent>(TestPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));
        if (HealthComp && HealthComp->GetHealth() > 0.0f)
        {
            bIsAnyBotAlive = true;
            break;
        }
    }

    if (!bIsAnyBotAlive)
    {
        SetWaveState(EWaveState::WaveComplete);
        PrepareForNextWave();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::CheckAnyPlayerAlive()
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->GetPawn())
        {
            APawn* MyPawn = PC->GetPawn();
            UTPSHealthComponent* HealthComp = Cast<UTPSHealthComponent>(MyPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));
            if (ensure(HealthComp) && (HealthComp->GetHealth() > 0.0f))
            {
                //A player is still alive
                return;
            }
        }
    }

    //No player alive
    GameOver();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::EndWave()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
    SetWaveState(EWaveState::WaitingToComplete);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::GameOver()
{
    EndWave();

    // @TODO: Finish up the match, present 'game over' to players
    SetWaveState(EWaveState::GameOver);
    UE_LOG(LogTemp, Log, TEXT("¡¡¡GAME OVER!!!"));
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::PrepareForNextWave()
{
    GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ATPSGameMode::StartWave, TimeBetweenWaves, false);
    SetWaveState(EWaveState::WaitingToStart);

    RestartDeadPlayers();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::RestartDeadPlayers()
{
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC && PC->GetPawn() == nullptr)
        {
            //If not got a Pawn the player is dead
            RestartPlayer(PC);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::SetWaveState(EWaveState NewState)
{
    ATPSGameState* GS = GetGameState<ATPSGameState>();
    if (ensureAlways(GS))
    {
        GS->SetWaveState(NewState);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::SpawnBotTimerElapsed()
{
    SpawnNewBot();

    NrOfBotsToSpawn--;
    if (NrOfBotsToSpawn <= 0)
    {
        EndWave();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSGameMode::StartWave()
{
    WaveCount++;
    NrOfBotsToSpawn = 2 * WaveCount;

    GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ATPSGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);
    SetWaveState(EWaveState::WaveInProgress);
}
