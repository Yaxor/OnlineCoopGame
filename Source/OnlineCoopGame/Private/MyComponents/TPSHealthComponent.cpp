//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "MyComponents/TPSHealthComponent.h"
#include "TPSGameMode.h"
#include "Net/UnrealNetwork.h"

//------------------------------------------------------------------------------------------------------------------------------------------
UTPSHealthComponent::UTPSHealthComponent()
{
    bIsDead   = false;
    MaxHealth = 100.0f;
    TeamNum   = 255;

    SetIsReplicatedByDefault(true);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UTPSHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    Health = MaxHealth;

    if (GetOwnerRole() == ROLE_Authority)
    {
        AActor* MyOwner = GetOwner();
        if (MyOwner)
        {
            MyOwner->OnTakeAnyDamage.AddDynamic(this, &UTPSHealthComponent::HandleTakeAnyDamage);
        }
    }
}

//-------------------------------------------------------PUBLIC FUNCTIONS-------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
float UTPSHealthComponent::GetHealth() const
{
    return Health;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UTPSHealthComponent::Heal(float HealAmount)
{
    if ((HealAmount <= 0.0f) || (Health <= 0.0f))
    {
        return;
    }

    Health = FMath::Clamp(Health + HealAmount, 0.0f, MaxHealth);

    UE_LOG(LogTemp, Warning, TEXT("Health Changed: %s (+%s)"), *FString::SanitizeFloat(Health), *FString::SanitizeFloat(HealAmount));

    OnHealthChanged.Broadcast(this, Health, (-HealAmount), nullptr, nullptr, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
bool UTPSHealthComponent::IsFriendly(AActor* ActorA, AActor* ActorB)
{
    if (ActorA == nullptr || ActorB == nullptr)
    {
        //Assume friendly
        return true;
    }

    UTPSHealthComponent* HealthCompA = Cast<UTPSHealthComponent>(ActorA->GetComponentByClass(UTPSHealthComponent::StaticClass()));
    UTPSHealthComponent* HealthCompB = Cast<UTPSHealthComponent>(ActorB->GetComponentByClass(UTPSHealthComponent::StaticClass()));

    if (HealthCompA == nullptr || HealthCompB == nullptr)
    {
        //Assume friendly
        return true;
    }

    return (HealthCompA->TeamNum == HealthCompB->TeamNum);
}

//-------------------------------------------------------PROTECTED FUNCTIONS----------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void UTPSHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
                                              class AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage <= 0.0f || bIsDead)
    {
        return;
    }

    //Check if the damage causer is my teammate or its myself
    if (DamageCauser != DamagedActor && IsFriendly(DamagedActor, DamageCauser))
    {
        return;
    }

    //Update health clamped
    Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

    bIsDead = (Health <= 0.0f);

    UE_LOG(LogTemp, Warning, TEXT("Health Changed: %s"), *FString::SanitizeFloat(Health));

    OnHealthChanged.Broadcast(this, Health, Damage, DamageType, InstigatedBy, DamageCauser);

    if (Health <= 0.0f)
    {
        ATPSGameMode* GM = Cast<ATPSGameMode>(GetWorld()->GetAuthGameMode());
        if (GM && DamageCauser && InstigatedBy)
        {
            GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, InstigatedBy);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UTPSHealthComponent::OnRep_Health(float OldHealth)
{
    float Damage = Health - OldHealth;

    OnHealthChanged.Broadcast(this, Health, Damage, nullptr, nullptr, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void UTPSHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UTPSHealthComponent, Health);
}