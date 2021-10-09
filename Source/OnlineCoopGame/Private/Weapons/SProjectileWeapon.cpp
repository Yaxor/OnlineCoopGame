//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "Weapons/SProjectileWeapon.h"

//------------------------------------------------------------------------------------------------------------------------------------------
void ASProjectileWeapon::StartFire()
{
    Fire();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ASProjectileWeapon::Fire()
{
    if (CurrentAmmo <= 0)
    {
        //No Ammo sound

        return;
    }

    AActor* MyOwner = GetOwner();
    if (MyOwner && ProjectileClass)
    {
        CurrentAmmo--;

        FVector  EyeLocation;
        FRotator EyeRotation;
        MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

        FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        if (ProjectileClass)
        {
            GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
        }
    }
}
