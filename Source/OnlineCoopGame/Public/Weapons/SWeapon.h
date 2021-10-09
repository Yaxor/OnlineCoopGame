//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "SWeapon.generated.h"

class UDamageType;
class UMatineeCameraShake;
class UParticleSystem;
class USkeletalMeshComponent;

//Contains info of a single hitscan weapon linetrace
USTRUCT()
struct FHitScanTrace
{
    GENERATED_BODY()

public:

    UPROPERTY()
    TEnumAsByte<EPhysicalSurface> SurfaceType;
    UPROPERTY()
    FVector_NetQuantize TraceEnd;
};

UCLASS()
class ONLINECOOPGAME_API ASWeapon : public AActor
{
    GENERATED_BODY()

public:
    //!Constructor
    ASWeapon();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED COMPONENTS & VARIABLES                                        *
    //*******************************************************************************************************************

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components)
    USkeletalMeshComponent* MeshComp;

    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    TSubclassOf<UMatineeCameraShake> FireCamShake;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    TSubclassOf<UDamageType> DamageType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    UParticleSystem* DefaultImpactVFX;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    UParticleSystem* FleshImpactVFX;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    UParticleSystem* MuzzleVFX;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    UParticleSystem* TraceVFX;

    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    FName MuzzleSocketName;
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = Weapon)
    FName TraceTargetName;

    FTimerHandle TimerHandle_Cadence;

    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    bool bInfiniteAmmo;

    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float BaseDamage;
    /*Bullet spread in degrees*/
    UPROPERTY(EditDefaultsOnly, Category = Weapon, meta = (ClampMin = 0.0f))
    float BulletSpread;
    /*Derived from FireRate*/
    float Cadence;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    /*RPM*/
    float FireRate;
    float LastFireTime;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    float HeadshotMultipler;

    UPROPERTY(Replicated)
    uint8 CurrentAmmo;
    UPROPERTY(EditDefaultsOnly, Category = Weapon)
    uint8 MaxAmmo;

    UPROPERTY(ReplicatedUsing = OnRep_HitScanTrace)
    FHitScanTrace HitScanTrace;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    void Reload();
    virtual void StartFire();
    void StopFire();

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    virtual void BeginPlay() override;
    /*Trace the world, from pawn eyes to crosshair location*/
    virtual void Fire();
    void PlayImpactFX(EPhysicalSurface SurfaceType, FVector ImpactPoint);
    /*Emit weapon fire effects*/
    void PlayFireFX(FVector TraceEndPoint);
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerFire();
    UFUNCTION(Server, Reliable, WithValidation)
    void ServerReload();

    UFUNCTION()
    void OnRep_HitScanTrace();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

};
