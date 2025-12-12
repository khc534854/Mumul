// Copyright Epic Games, Inc. All Rights Reserved.

#include "MumulCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Player/CuteAlienAnim.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AMumulCharacter

AMumulCharacter::AMumulCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.25f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	static ConstructorHelpers::FObjectFinder<UAnimMontage> JumpMontageFinder(
		TEXT("/Game/Yeomin/Characters/CuteAlien/Animations/Animation2/Jump_Montage.Jump_Montage"));
	if (JumpMontageFinder.Succeeded())
	{
		JumpMontage = JumpMontageFinder.Object;
	}
}

void AMumulCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	
	PlayerAnim = Cast<UCuteAlienAnim>(GetMesh()->GetAnimInstance());
}

void AMumulCharacter::SetFirstPersonView(bool bEnable)
{
	if (bEnable)
	{
		// 1인칭 모드: 카메라 붐 길이를 0으로 줄임
		CameraBoom->TargetArmLength = 30.0f;
        
		// 카메라 위치를 약간 위로 조정 (캐릭터 눈높이)
		// 필요에 따라 Z값을 조정하세요 (예: 20~60)
		CameraBoom->SocketOffset = FVector(0, 0, 80.f); 

		// 캐릭터가 카메라 회전을 따라가도록 설정 (설치 편의성)
		bUseControllerRotationYaw = true;
		GetMesh()->SetOwnerNoSee(true);
	}
	else
	{
		// 3인칭 모드: 원래 설정으로 복구
		CameraBoom->TargetArmLength = 400.0f; // 생성자 기본값
		CameraBoom->SocketOffset = FVector::ZeroVector;

		// 자유로운 카메라 회전 복구
		bUseControllerRotationYaw = false;
		GetMesh()->SetOwnerNoSee(false);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMumulCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this,
										   &AMumulCharacter::Server_OnJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
										   &AMumulCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
										   &AMumulCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AMumulCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMumulCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMumulCharacter::Server_OnJump_Implementation(const FInputActionValue& Value)
{
	if (GetCharacterMovement()->IsFalling())
	{
		Multicast_OnRollAnimation();
		return;
	}
	
	Multicast_OnJumpAnimation();
}

void AMumulCharacter::Multicast_OnJumpAnimation_Implementation()
{
	if (PlayerAnim->Montage_IsPlaying(JumpMontage) == false && GetCharacterMovement()->IsFalling() == false)
	{
		PlayerAnim->Montage_Play(JumpMontage);
	}
}

void AMumulCharacter::Multicast_OnRollAnimation_Implementation()
{
	if (PlayerAnim->Montage_IsPlaying(RollMontage) == false)
	{
		LaunchCharacter(GetActorForwardVector() * RollStrength, false, false);
		PlayerAnim->Montage_Play(RollMontage);
	}
}