//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "TPSExplosiveBarrel.h"
#include "Net/UnrealNetwork.h"

#include "Kismet/GameplayStatics.h"

#include "MyComponents/TPSHealthComponent.h"

#include "PhysicsEngine/RadialForceComponent.h"


//------------------------------------------------------------------------------------------------------------------------------------------
ATPSExplosiveBarrel::ATPSExplosiveBarrel()
{
    HealthComp = CreateDefaultSubobject<UTPSHealthComponent>("HealthComp");
    HealthComp->OnHealthChanged.AddDynamic(this, &ATPSExplosiveBarrel::OnHealthChanged);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
    MeshComp->SetSimulatePhysics(true);
    MeshComp->SetCollisionObjectType(ECC_PhysicsBody); //Set to physics body to let radial component affects us (eg. when a nearby barrel explodes)
    RootComponent = MeshComp;

    RadialForceComp = CreateDefaultSubobject<URadialForceComponent>("RadialForceComp");
    RadialForceComp->SetupAttachment(MeshComp);
    RadialForceComp->Radius             = 250.0f;
    RadialForceComp->bImpulseVelChange  = true;
    RadialForceComp->bAutoActivate      = false; //Prevent component from ticking, and only use FireImpulse() instead
    RadialForceComp->bIgnoreOwningActor = true;

    ExplosionImpulse = 400.0f;

    SetReplicates(true);
    SetReplicateMovement(true);
}

//-------------------------------------------------------PROTECTED FUNCTIONS----------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSExplosiveBarrel::OnHealthChanged(UTPSHealthComponent* HealthComponent, float Health, float HealthDelta,
                                          const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (bExploded)
    {
        return;
    }

    if (Health <= 0.0f)
    {
        bExploded = true;
        OnRep_Exploded();

        //Boost barrel upwards
        FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
        MeshComp->AddImpulse(BoostIntensity, NAME_None, true);

        //Blast away nearby physics actors
        RadialForceComp->FireImpulse();

        //Apply radial damage
        Explode();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSExplosiveBarrel::OnRep_Exploded()
{
    //Play VFX and change material
    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVFX, GetActorLocation());
    MeshComp->SetMaterial(0, ExplodedMaterial);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATPSExplosiveBarrel, bExploded);
}
