//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TPSTrackerBot.generated.h"

class ACharacter;
class ATPSCharacter;
class UParticleSystem;
class USphereComponent;
class UStaticMeshComponent;
class USoundCue;
class UTPSHealthComponent;

UCLASS()
class ONLINECOOPGAME_API ATPSTrackerBot : public APawn
{
    GENERATED_BODY()

public:
    //!Constructor
    ATPSTrackerBot(const FObjectInitializer& ObjectInitializer);

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS & VARIABLES                                        *
    //*******************************************************************************************************************

    UPROPERTY(VisibleDefaultsOnly, Category = Components)
    UStaticMeshComponent* MeshComp;
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
    UTPSHealthComponent*  HealthComp;
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
    USphereComponent*     SphereComp;


    //Dynamic material to pulse on hit
    UMaterialInstanceDynamic* MatInst;

    FVector NextPathPoint;

    FTimerHandle TimerHandle_SelfDamage;
    FTimerHandle TimerHandle_RefreshPath;

    UPROPERTY(EditDefaultsOnly, Category = "VFX")
    UParticleSystem* ExplosionVFX;

    UPROPERTY(EditDefaultsOnly, Category = "SFX")
    USoundCue* ExplodeSFX;
    UPROPERTY(EditDefaultsOnly, Category = "SFX")
    USoundCue* SelfDestructSFX;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    TSubclassOf<ATPSCharacter> PlayerBP;

    ACharacter* TargetPlayerPawn;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    bool bUseVelocityChange;
    bool bExploded;
    //Set if prefer a nearest target or a random target
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    bool bPreferRandomTarget;
    bool bStartedSelfDestruction;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float ExplotionDamage;
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float ExplotionRadius;
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float MaxVelocity;
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float MovementForce;
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float RequiredDistance;
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    float SelfDamageInterval;

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    int32 MaxPowerLevel;
    UPROPERTY(ReplicatedUsing = OnRep_OnNearbyBots)
    int32 PowerLevel;

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
    virtual void Tick(float DeltaTime) override;

    void DamageSelf();
    void SeftDestruct();
    /*Find nearby bots and grow the power level based on the amount*/
    void OnCheckNearbyBots();
    void RefreshPath();

    FVector GetNextPathPoint();

    UFUNCTION()
    void OnRep_OnNearbyBots();

    UFUNCTION()
    void OnHealthChanged(UTPSHealthComponent* HealthComponent,
                         float Health,
                         float HealthDelta,
                         const class UDamageType* DamageType,
                         class AController* InstigatedBy,
                         AActor* DamageCauser);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
