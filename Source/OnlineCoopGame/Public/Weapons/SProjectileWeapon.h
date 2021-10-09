//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "Weapons/SWeapon.h"
#include "SProjectileWeapon.generated.h"

UCLASS()
class ONLINECOOPGAME_API ASProjectileWeapon : public ASWeapon
{
    GENERATED_BODY()

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED VARIABLES                                                     *
    //*******************************************************************************************************************

    UPROPERTY(EditDefaultsOnly, Category = ProjectileWeapon)
    TSubclassOf<AActor> ProjectileClass;

    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    virtual void StartFire() override;
    virtual void Fire() override;
};
