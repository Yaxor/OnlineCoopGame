//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#include "Weapons/SWeapon.h"
#include "Net/UnrealNetwork.h"

#include "Components/SkeletalMeshComponent.h"

#include "DrawDebugHelpers.h"

#include "Kismet/GameplayStatics.h"

#include "OnlineCoopGame.h"

#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

#include "PhysicalMaterials/PhysicalMaterial.h"

#include "TimerManager.h"

//------------------------------------------------------------------------------------------------------------------------------------------
static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(TEXT("COOP.DebugWeapons"), DebugWeaponDrawing,
                                               TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);

//------------------------------------------------------------------------------------------------------------------------------------------
ASWeapon::ASWeapon()
{
    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComp");
    RootComponent = MeshComp;

    MuzzleSocketName      = "MuzzleSocket";
    TraceTargetName       = "Target";
    bInfiniteAmmo         = false;
    BaseDamage            = 20.0f;
    BulletSpread          = 2.0f;
    FireRate              = 600.0f;
    HeadshotMultipler     = 4.0f;
    MaxAmmo               = 30;
    NetUpdateFrequency    = 66.0f;
    MinNetUpdateFrequency = 33.0f;

    SetReplicates(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::BeginPlay()
{
    Super::BeginPlay();

    Cadence     = 60 / FireRate;
    CurrentAmmo = MaxAmmo;
}

//-------------------------------------------------------PUBLIC FUNCTIONS-------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::Reload()
{
    //If you are a Client, send a request to Server
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerReload();
        return;
    }

    if (CurrentAmmo != MaxAmmo)
    {
        //Reload Feedback

        CurrentAmmo = MaxAmmo;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::StartFire()
{
    float FirstDelay = FMath::Max(LastFireTime + Cadence - GetWorld()->TimeSeconds, 0.0f);

    GetWorldTimerManager().SetTimer(TimerHandle_Cadence, this, &ASWeapon::Fire, Cadence, true, FirstDelay);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::StopFire()
{
    GetWorldTimerManager().ClearTimer(TimerHandle_Cadence);
}

//-------------------------------------------------------PROTECTED FUNCTIONS----------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::Fire()
{
    //If you are a Client, send a request to Server
    if (GetLocalRole() < ROLE_Authority)
    {
        ServerFire();
    }

    if (CurrentAmmo <= 0 && !bInfiniteAmmo)
    {
        return;
    }

    AActor* MyOwner = GetOwner();

    if (MyOwner && (GetLocalRole() == ROLE_Authority))
    {
        if (!bInfiniteAmmo)
        {
            CurrentAmmo--;
        }

        FVector  EyeLocation;
        FRotator EyeRotation;
        MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

        FVector ShotDirection = EyeRotation.Vector();

        //Bullet Spread
        float HalfRad = FMath::DegreesToRadians(BulletSpread);
        ShotDirection = FMath::VRandCone(ShotDirection, HalfRad, HalfRad);

        FVector EndLocation   = EyeLocation + (ShotDirection * 10000);

        FCollisionQueryParams Queryparams;
        Queryparams.AddIgnoredActor(MyOwner);
        Queryparams.AddIgnoredActor(this);
        Queryparams.bTraceComplex           = true;
        Queryparams.bReturnPhysicalMaterial = true;

        // Particle Target parameter
        FVector TraceEndPoint = EndLocation;

        EPhysicalSurface SurfaceType = SurfaceType_Default;

        FHitResult Hit;
        if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, EndLocation, COLLISION_WEAPON, Queryparams))
        {
            AActor* HitActor = Hit.GetActor();

            SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());

            float ActualDamage = BaseDamage;
            if (SurfaceType == SURFACE_FLESHVULNERABLE)
            {
                ActualDamage *= HeadshotMultipler;
            }

            UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit,
                                               MyOwner->GetInstigatorController(), MyOwner, DamageType);

            TraceEndPoint = Hit.ImpactPoint;

            HitScanTrace.SurfaceType = SurfaceType;
        }

        if (DebugWeaponDrawing > 0)
        {
            DrawDebugLine(GetWorld(), EyeLocation, EndLocation, FColor::Red, false, 1.0f, 0, 1.0f);
        }

        PlayFireFX(TraceEndPoint);
        PlayImpactFX(SurfaceType, Hit.ImpactPoint);
        HitScanTrace.SurfaceType = SurfaceType;
        HitScanTrace.TraceEnd = TraceEndPoint;

        LastFireTime = GetWorld()->TimeSeconds;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::PlayImpactFX(EPhysicalSurface SurfaceType, FVector ImpactPoint)
{
    //Impact FX
    UParticleSystem* SelectedVFX = nullptr;
    switch (SurfaceType)
    {
    case SURFACE_FLESHDEFAULT:
    case SURFACE_FLESHVULNERABLE:
        SelectedVFX = FleshImpactVFX;
        break;
    default:
        SelectedVFX = DefaultImpactVFX;
        break;
    }

    if (SelectedVFX)
    {
        FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

        FVector ShotDirection  = ImpactPoint - MuzzleLocation;
        ShotDirection.Normalize();

        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedVFX, ImpactPoint, ShotDirection.Rotation());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::PlayFireFX(FVector TraceEndPoint)
{
    if (MuzzleVFX)
    {
        UGameplayStatics::SpawnEmitterAttached(MuzzleVFX, MeshComp, MuzzleSocketName);
    }

    if (TraceVFX)
    {
        FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

        UParticleSystemComponent* TraceComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TraceVFX, MuzzleLocation);

        if (TraceComp)
        {
            TraceComp->SetVectorParameter(TraceTargetName, TraceEndPoint);
        }
    }

    APawn* MyOwner = Cast<APawn>(GetOwner());
    if (MyOwner)
    {
        APlayerController* PlayerController = Cast<APlayerController>(MyOwner->GetController());
        if (PlayerController)
        {
            PlayerController->ClientStartCameraShake(FireCamShake);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::ServerFire_Implementation()
{
    Fire();
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool ASWeapon::ServerFire_Validate()
{
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::ServerReload_Implementation()
{
    Reload();
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool ASWeapon::ServerReload_Validate()
{
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::OnRep_HitScanTrace()
{
    //Play cosmetic FX
    PlayFireFX(HitScanTrace.TraceEnd);
    PlayImpactFX(HitScanTrace.SurfaceType, HitScanTrace.TraceEnd);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASWeapon, CurrentAmmo);
    DOREPLIFETIME(ASWeapon, HitScanTrace);
    //DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
}
