//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "AI/TPSTrackerBot.h"
#include "Player/TPSCharacter.h"
#include "Net/UnrealNetwork.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MyComponents/TPSHealthComponent.h"

#include "DrawDebugHelpers.h"

#include "GameFramework/Character.h"

#include "Kismet/GameplayStatics.h"

#include "NavigationPath.h"
#include "NavigationSystem.h"

#include "Sound/SoundCue.h"

//------------------------------------------------------------------------------------------------------------------------------------------
static int32 DebugTrackerBotDrawing = 0;
FAutoConsoleVariableRef CVARDebugTrackerBotDrawing(TEXT("COOP.DebugTrackerBot"), DebugTrackerBotDrawing,
                                                   TEXT("Draw Debug Lines for TrackerBot"), ECVF_Cheat);

//------------------------------------------------------------------------------------------------------------------------------------------
ATPSTrackerBot::ATPSTrackerBot(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
    MeshComp->SetCanEverAffectNavigation(false);
    MeshComp->SetSimulatePhysics(true);
    RootComponent = MeshComp;

    HealthComp = CreateDefaultSubobject<UTPSHealthComponent>("HealthComp");
    HealthComp->OnHealthChanged.AddDynamic(this, &ATPSTrackerBot::OnHealthChanged);

    SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
    SphereComp->SetSphereRadius(200.0f);
    SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    SphereComp->SetupAttachment(RootComponent);

    bUseVelocityChange  = true;
    bPreferRandomTarget = true;
    ExplotionDamage     = 40.0f;
    ExplotionRadius     = 300.0f;
    MaxVelocity         = 1000.0f;
    MovementForce       = 2000.0f;
    RequiredDistance    = 100.0f;
    SelfDamageInterval  = 0.25f;
    MaxPowerLevel       = 4;
}

//-------------------------------------------------------PUBLIC FUNCTIONS-------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (!bStartedSelfDestruction && !bExploded)
    {
        ATPSCharacter* PlayerPawn = Cast<ATPSCharacter>(OtherActor);
        if (PlayerPawn && !UTPSHealthComponent::IsFriendly(OtherActor, this))
        {
            if (GetLocalRole() == ROLE_Authority)
            {
                //Start self destruction sequence
                GetWorldTimerManager().SetTimer(TimerHandle_SelfDamage, this, &ATPSTrackerBot::DamageSelf, SelfDamageInterval, true, 0.0f);
            }

            bStartedSelfDestruction = true;

            UGameplayStatics::SpawnSoundAttached(SelfDestructSFX, RootComponent);
        }
    }
}

//-------------------------------------------------------PROTECTED FUNCTIONS----------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSTrackerBot::BeginPlay()
{
    Super::BeginPlay();

    if (GetLocalRole() == ROLE_Authority)
    {
        NextPathPoint = GetNextPathPoint();

        //Every one second check for nearby bots
        FTimerHandle Timerhandle_CheckPowerLevel;
        GetWorldTimerManager().SetTimer(Timerhandle_CheckPowerLevel, this, &ATPSTrackerBot::OnCheckNearbyBots, 1.0f, true);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSTrackerBot::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if ((GetLocalRole() == ROLE_Authority) && (!bExploded))
    {
        float DistanceToTarget = (GetActorLocation() - NextPathPoint).Size();

        if (DistanceToTarget <= RequiredDistance)
        {
            NextPathPoint = GetNextPathPoint();

            if (DebugTrackerBotDrawing)
            {
                DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
            }
        }
        else
        {
            //Keep moving towards next target
            FVector ForceDirection = NextPathPoint - GetActorLocation();
            ForceDirection.Normalize();
            ForceDirection *= MovementForce;

            MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
            FVector CurrentVelocity = MeshComp->GetPhysicsLinearVelocity();
            FVector ClampedVelocity = CurrentVelocity.GetClampedToMaxSize(MaxVelocity);
            MeshComp->SetPhysicsLinearVelocity(ClampedVelocity);

            if (DebugTrackerBotDrawing)
            {
                DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
            }
        }

        if (DebugTrackerBotDrawing)
        {
            DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSTrackerBot::DamageSelf()
{
    UGameplayStatics::ApplyDamage(this, 20.0f, GetInstigatorController(), this, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSTrackerBot::SeftDestruct()
{
    if (bExploded)
    {
        return;
    }

    bExploded = true;

    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionVFX, GetActorLocation());
    UGameplayStatics::PlaySoundAtLocation(this, ExplodeSFX, GetActorLocation());

    MeshComp->SetVisibility(false, true);
    MeshComp->SetSimulatePhysics(false);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (GetLocalRole() == ROLE_Authority)
    {
        TArray<AActor*> IgnoredActors;
        IgnoredActors.Add(this);

        //Increase damage based on the power level
        float ActualDamage = ExplotionDamage + (ExplotionDamage * PowerLevel);

        //Make radial damage
        UGameplayStatics::ApplyRadialDamage(this, ExplotionDamage, GetActorLocation(), ExplotionRadius,
                                            nullptr, IgnoredActors, this, GetInstigatorController(), true);

        if (DebugTrackerBotDrawing)
        {
            DrawDebugSphere(GetWorld(), GetActorLocation(), ExplotionRadius, 12, FColor::Magenta, false, 2.0f, 0, 1.0f);
        }

        SetLifeSpan(2.0f);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSTrackerBot::OnCheckNearbyBots()
{
    //Distance to check for nearby bots
    const float Radius = 600.0f;

    //Create a temporary collision shape for overlaps
    FCollisionShape CollShape;
    CollShape.SetSphere(Radius);

    FCollisionObjectQueryParams QueryParams;
    QueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
    QueryParams.AddObjectTypesToQuery(ECC_Pawn);

    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, QueryParams, CollShape);

    if (DebugTrackerBotDrawing)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation(), Radius, 12, FColor::White, false, 1.0f);
    }

    int32 NrOfBots = 0;
    for (FOverlapResult Result : Overlaps)
    {
        //Check if overlapped with another bot
        ATPSTrackerBot* Bot = Cast<ATPSTrackerBot>(Result.GetActor());

        //Ignore this bot
        if (Bot && Bot != this)
        {
            NrOfBots++;
        }
    }

    PowerLevel = FMath::Clamp(NrOfBots, 0, MaxPowerLevel);

    if (MatInst == nullptr)
    {
        MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
    }

    if (MatInst)
    {
        float Alpha = PowerLevel / (float)MaxPowerLevel;

        MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
    }

    if (DebugTrackerBotDrawing)
    {
        DrawDebugString(GetWorld(), FVector::ZeroVector, FString::FromInt(PowerLevel), this, FColor::White, 1.0f, true);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSTrackerBot::RefreshPath()
{
    NextPathPoint = GetNextPathPoint();
}

//------------------------------------------------------------------------------------------------------------------------------------------
FVector ATPSTrackerBot::GetNextPathPoint()
{
    if (!bPreferRandomTarget) //If prefer a nearest target
    {
        AActor* BestTarget = nullptr;
        float NearestTargetDistance = FLT_MAX;

        for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
        {
            APawn* TestPawn = It->Get();
            if (TestPawn == nullptr || UTPSHealthComponent::IsFriendly(TestPawn, this))
            {
                continue;
            }

            UTPSHealthComponent* TestPawnHealthComp = Cast<UTPSHealthComponent>(TestPawn->GetComponentByClass(UTPSHealthComponent::StaticClass()));
            if (TestPawnHealthComp && TestPawnHealthComp->GetHealth() > 0.0f)
            {
                float DistanceToTestPawn = (TestPawn->GetActorLocation() - GetActorLocation()).Size();

                if (DistanceToTestPawn < NearestTargetDistance)
                {
                    BestTarget = TestPawn;
                    NearestTargetDistance = DistanceToTestPawn;
                }
            }
        }

        if (BestTarget)
        {
            UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), BestTarget);

            GetWorldTimerManager().ClearTimer(TimerHandle_RefreshPath);
            GetWorldTimerManager().SetTimer(TimerHandle_RefreshPath, this, &ATPSTrackerBot::RefreshPath, 5.0f, false);

            if (NavPath->PathPoints.Num() > 1)
            {
                //Return next point
                return NavPath->PathPoints[1];
            }
        }
    }
    else //If prefer a random target
    {
        //If the target player is null, try to find one
        if (TargetPlayerPawn == nullptr)
        {
            //Get host player
            TargetPlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

            //Get all players
            TArray<AActor*> Players;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), PlayerBP, Players);

            //Make a random number between 0 and the number of players
            int NrPlayers = (Players.Num() - 1);
            int RandomPlayerIndex = FMath::RandRange(0, NrPlayers);

            TargetPlayerPawn = UGameplayStatics::GetPlayerCharacter(this, RandomPlayerIndex);

            if (DebugTrackerBotDrawing)
            {
                UE_LOG(LogTemp, Warning, TEXT("NrPlayers: %s"), *FString::FromInt(NrPlayers));
                UE_LOG(LogTemp, Warning, TEXT("RandomPlayerIndex: %s"), *FString::FromInt(RandomPlayerIndex));
            }
        }

        if (TargetPlayerPawn)
        {
            UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), TargetPlayerPawn);

            if (NavPath->PathPoints.Num() > 1)
            {
                //Return next point
                return NavPath->PathPoints[1];
            }
        }
    }

    //Failed to find path
    return GetActorLocation();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSTrackerBot::OnRep_OnNearbyBots()
{
    if (MatInst == nullptr)
    {
        MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
    }

    if (MatInst)
    {
        float Alpha = PowerLevel / (float)MaxPowerLevel;

        MatInst->SetScalarParameterValue("PowerLevelAlpha", Alpha);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSTrackerBot::OnHealthChanged(UTPSHealthComponent* HealthComponent, float Health, float HealthDelta, 
                                     const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if (MatInst == nullptr)
    {
        MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));
    }

    if (MatInst)
    {
        MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
    }

    if (DebugTrackerBotDrawing)
    {
        UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
    }

    //Explode on Health Points <= 0
    if (Health <= 0.0f)
    {
        SeftDestruct();
    }
}

void ATPSTrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATPSTrackerBot, PowerLevel);
}
