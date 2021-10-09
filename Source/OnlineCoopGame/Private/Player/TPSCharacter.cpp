//+--------------------------------------------------------+
//| Project    : OnlineCoopGame                            |
//| UE Version : 4.26.2                                    |
//| Author     : Matias Till                               |
//+--------------------------------------------------------+


#include "Player/TPSCharacter.h"
#include "Net/UnrealNetwork.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "MyComponents/TPSHealthComponent.h"
#include "OnlineCoopGame.h"
#include "Weapons/SWeapon.h"

//------------------------------------------------------------------------------------------------------------------------------------------
ATPSCharacter::ATPSCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>("SpringArmComp");
    SpringArmComp->SetupAttachment(RootComponent);
    SpringArmComp->bUsePawnControlRotation = true;

    CameraComp = CreateDefaultSubobject<UCameraComponent>("CameraComp");
    CameraComp->SetupAttachment(SpringArmComp);

    HealthComp = CreateDefaultSubobject<UTPSHealthComponent>("HealthComp");

    GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

    GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

    ZoomFOV         = 65.0;
    ZoomInterpSpeed = 22.0f;

    WeaponSocket = "WeaponSocket";
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::BeginPlay()
{
    Super::BeginPlay();

    DefaultFOV = CameraComp->FieldOfView;
    HealthComp->OnHealthChanged.AddDynamic(this, &ATPSCharacter::OnHealthChanged);

    if (GetLocalRole() == ROLE_Authority)
    {
        //Spawn a default weapon
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(DefaultWeapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (CurrentWeapon)
        {
            CurrentWeapon->SetOwner(this);
            CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponSocket);
        }
    }
}

//-------------------------------------------------------PUBLIC FUNCTIONS-------------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ZoomInterp(DeltaTime);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("LookUp"     , this, &ATPSCharacter::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("MoveForward", this, &ATPSCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight"  , this, &ATPSCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn"       , this, &ATPSCharacter::AddControllerYawInput);

    PlayerInputComponent->BindAction("Crouch", IE_Pressed , this, &ATPSCharacter::BeginCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ATPSCharacter::EndCrouch);
    PlayerInputComponent->BindAction("Fire"  , IE_Pressed , this, &ATPSCharacter::StartFireWeapon);
    PlayerInputComponent->BindAction("Fire"  , IE_Released, this, &ATPSCharacter::StopFireWeapon);
    PlayerInputComponent->BindAction("Jump"  , IE_Pressed , this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Reload", IE_Pressed , this, &ATPSCharacter::ReloadWeapon);
    PlayerInputComponent->BindAction("Zoom"  , IE_Pressed , this, &ATPSCharacter::BeginZoom);
    PlayerInputComponent->BindAction("Zoom"  , IE_Released, this, &ATPSCharacter::EndZoom);
}

//------------------------------------------------------------------------------------------------------------------------------------------
FVector ATPSCharacter::GetPawnViewLocation() const
{
    if (CameraComp)
    {
        return CameraComp->GetComponentLocation();
    }

    return Super::GetPawnViewLocation();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::StartFireWeapon()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StartFire();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::StopFireWeapon()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->StopFire();
    }
}

//-------------------------------------------------------PROTECTED FUNCTIONS----------------------------------------------------------------

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::BeginCrouch()
{
    Crouch();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::BeginZoom()
{
    bWantsToZoom = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::EndCrouch()
{
    UnCrouch();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::EndZoom()
{
    bWantsToZoom = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::MoveForward(float Value)
{
    AddMovementInput(GetActorForwardVector() * Value);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::MoveRight(float Value)
{
    AddMovementInput(GetActorRightVector() * Value);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::ReloadWeapon()
{
    if (CurrentWeapon)
    {
        CurrentWeapon->Reload();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::ZoomInterp(const float DeltaTime)
{
    const float TargetFOV = bWantsToZoom ? ZoomFOV : DefaultFOV;

    float CurrentFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

    CameraComp->SetFieldOfView(CurrentFOV);
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::OnHealthChanged(UTPSHealthComponent* HealthComponent, float Health, float HealthDelta,
                                    const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
    if ((Health <= 0.0f) && !bDied)
    {
        //Die!
        bDied = true;

        GetMovementComponent()->StopMovementImmediately();
        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        DetachFromControllerPendingDestroy();
        OnRep_Died();
        SetLifeSpan(10.0f);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::OnRep_Died()
{
    StopFireWeapon();
}

//------------------------------------------------------------------------------------------------------------------------------------------
void ATPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATPSCharacter, CurrentWeapon);
    DOREPLIFETIME(ATPSCharacter, bDied);
}
