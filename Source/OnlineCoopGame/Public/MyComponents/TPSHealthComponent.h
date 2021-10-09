//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "TPSHealthComponent.generated.h"

//On hange health event
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UTPSHealthComponent*, HealthComp, float, Health, float, HealthDelta, const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS( ClassGroup=(COOP), meta=(BlueprintSpawnableComponent) )
class ONLINECOOPGAME_API UTPSHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    //!Constructor
    UTPSHealthComponent();

    //*******************************************************************************************************************
    //                                          PUBLIC VARIABLES                                                        *
    //*******************************************************************************************************************

    UPROPERTY(BlueprintAssignable, Category = Events)
    //Delegate to make damage to my owner
    FOnHealthChangedSignature OnHealthChanged;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = HealthComponent)
    uint8 TeamNum;

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED VARIABLES                                                     *
    //*******************************************************************************************************************

    bool bIsDead;

    UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = HealthComponent)
    float Health;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = HealthComponent)
    float MaxHealth;

public:
    //*******************************************************************************************************************
    //                                          PUBLIC FUNCTIONS                                                        *
    //*******************************************************************************************************************

    float GetHealth() const;

    UFUNCTION(BlueprintCallable, Category = "HealthComponent")
    void Heal(float HealAmount);

    /*Compare the ActorA with ActorB to see if they are companions*/
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "HealthComponent")
    static bool IsFriendly(AActor* ActorA, AActor*ActorB);

protected:
    //*******************************************************************************************************************
    //                                          PROTECTED FUNCTIONS                                                     *
    //*******************************************************************************************************************

    virtual void BeginPlay() override;

    UFUNCTION()
    void HandleTakeAnyDamage(AActor* DamagedActor,
                             float Damage,
                             const class UDamageType* DamageType,
                             class AController* InstigatedBy,
                             AActor* DamageCauser);

    UFUNCTION()
    void OnRep_Health(float OldHealth);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
