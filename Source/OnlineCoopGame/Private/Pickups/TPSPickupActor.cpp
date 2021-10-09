//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "Pickups/TPSPickupActor.h"
#include "Player/TPSCharacter.h"
#include "Powerups/TPSPowerupActor.h"

#include "Components/SphereComponent.h"
#include "Components/DecalComponent.h"

//------------------------------------------------------------------------------------------------------------------------------------------
ATPSPickupActor::ATPSPickupActor()
{
    SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
    SphereComp->SetSphereRadius(75.0f);
    RootComponent = SphereComp;

    DecalComp = CreateDefaultSubobject<UDecalComponent>("DecalComp");
    DecalComp->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f));
    DecalComp->DecalSize = FVector(64.0f, 75.0f, 75.0f);
    DecalComp->SetupAttachment(RootComponent);

    CoolDownDuration = 10.0f;

    SetReplicates(true);
}

//-------------------------------------------------------PUBLIC FUNCTIONS-------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSPickupActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    ATPSCharacter* PlayerPawn = Cast<ATPSCharacter>(OtherActor);
    if (PlayerPawn)
    {
        if ((GetLocalRole() == ROLE_Authority) && PowerUpInstance)
        {
            PowerUpInstance->ActivatePowerup(OtherActor);
            PowerUpInstance = nullptr;

            //Set timer to respawn
            GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &ATPSPickupActor::Respawn, CoolDownDuration);
        }
    }
}

//-------------------------------------------------------PROTECTED FUNCTIONS----------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSPickupActor::BeginPlay()
{
    Super::BeginPlay();

    if (GetLocalRole() == ROLE_Authority)
    {
        Respawn();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSPickupActor::Respawn()
{
    if (PowerUpClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("PowerUpClass is nullptr in %s. Please update your Blueprint"), *GetName());
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    PowerUpInstance = GetWorld()->SpawnActor<ATPSPowerupActor>(PowerUpClass, GetTransform(), SpawnParams);
}
