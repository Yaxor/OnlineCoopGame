//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "TPSExplosiveBarrel.generated.h"

class UParticleSystem;
class URadialForceComponent;
class UStaticMeshComponent;
class UTPSHealthComponent;

UCLASS()
class ONLINECOOPGAME_API ATPSExplosiveBarrel : public AActor
{
    GENERATED_BODY()

public:
    //!Constructor
    ATPSExplosiveBarrel();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS & VARIABLES                                        *
    //*******************************************************************************************************************

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
    URadialForceComponent*   RadialForceComp;
    UPROPERTY(VisibleAnywhere, Category = Components)
    UStaticMeshComponent*    MeshComp;
    UPROPERTY(VisibleAnywhere, Category = Components)
    UTPSHealthComponent*     HealthComp;

    /*Particle to play when health reached zero*/
    UPROPERTY(EditDefaultsOnly, Category = FX)
    UParticleSystem* ExplosionVFX;

    /*The material to replace the original on the mesh once exploded*/
    UPROPERTY(EditDefaultsOnly, Category = FX)
    UMaterialInterface* ExplodedMaterial;

    UPROPERTY(ReplicatedUsing = OnRep_Exploded)
    bool bExploded;

    /*Impulse applied to the barrel mesh when it explodes to boost it up*/
    UPROPERTY(EditDefaultsOnly, Category = FX)
    float ExplosionImpulse;

    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    UFUNCTION()
    void OnHealthChanged(UTPSHealthComponent* HealthComponent,
                         float Health,
                         float HealthDelta,
                         const class UDamageType* DamageType,
                         class AController* InstigatedBy,
                         AActor* DamageCauser);

    UFUNCTION(BlueprintImplementableEvent)
    //Apply radial damage
    void Explode();

    UFUNCTION()
    void OnRep_Exploded();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
