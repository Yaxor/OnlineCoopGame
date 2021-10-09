//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPSPickupActor.generated.h"

class ATPSPowerupActor;
class UDecalComponent;
class USphereComponent;

UCLASS()
class ONLINECOOPGAME_API ATPSPickupActor : public AActor
{
    GENERATED_BODY()

public:
    //!Constructor
    ATPSPickupActor();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS & VARIABLES                                        *
    //*******************************************************************************************************************

    UPROPERTY(VisibleAnywhere, Category = "Components")
    UDecalComponent*  DecalComp;
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* SphereComp;

    UPROPERTY(EditInstanceOnly, Category = "PickupActor")
    TSubclassOf<ATPSPowerupActor> PowerUpClass;
    ATPSPowerupActor* PowerUpInstance;

    FTimerHandle TimerHandle_RespawnTimer;

    UPROPERTY(EditInstanceOnly, Category = "PickupActor")
    float CoolDownDuration;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    virtual void BeginPlay() override;
    void Respawn();
};
