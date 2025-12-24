#include "Object/TendencyActor.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h" // 텍스트 변경용
#include "Blueprint/UserWidget.h"
#include "Data/FTendencyDialogueData.h"
#include "Kismet/KismetMathLibrary.h" // 랜덤 함수용

ATendencyActor::ATendencyActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    InteractionUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionUI"));
    InteractionUI->SetupAttachment(RootComponent);
    InteractionUI->SetWidgetSpace(EWidgetSpace::Screen);
    InteractionUI->SetVisibility(false);
    InteractionUI->SetIsReplicated(false); 
}

void ATendencyActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (GetCapsuleComponent())
    {
       GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ATendencyActor::OnOverlapBegin);
       GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ATendencyActor::OnOverlapEnd);
    }

    if (InteractionUI)
    {
       InteractionUI->SetVisibility(false);
    }
    
    if (IdleMontage)
    {
       GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
       GetMesh()->AnimationData.bSavedLooping = true;
       GetMesh()->AnimationData.bSavedPlaying = true;
       GetMesh()->AnimationData.AnimToPlay = IdleMontage;
    }
    UpdateBodyMaterial(TendencyIndex);
}

void ATendencyActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ATendencyActor::UpdateBodyMaterial(int32 TendencyIdx)
{
    if (PlayerBodyMaterials.Num() == 0) return;

    int32 MatIndexStart = 0;

    if (TendencyIdx <= 1) MatIndexStart = 0;
    else if (TendencyIdx >= 2 && TendencyIdx <= 5) MatIndexStart = (TendencyIdx - 1) * 3;

    if (PlayerBodyMaterials.IsValidIndex(MatIndexStart + 2))
    {
       GetMesh()->SetMaterial(0, PlayerBodyMaterials[MatIndexStart]);
       GetMesh()->SetMaterial(2, PlayerBodyMaterials[MatIndexStart + 1]);
       GetMesh()->SetMaterial(3, PlayerBodyMaterials[MatIndexStart + 2]);
    }
}

void ATendencyActor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// [핵심] 랜덤 대사 추출 및 UI 적용 함수
void ATendencyActor::SetRandomDialogue()
{
    if (!DialogueDataTable) return;

    // 1. 데이터 테이블의 모든 행 가져오기
    TArray<FTendencyDialogueData*> AllRows;
    DialogueDataTable->GetAllRows<FTendencyDialogueData>(TEXT("Query Context"), AllRows);

    // 2. 현재 TendencyIndex와 일치하는 대사만 필터링
    TArray<FString> FilteredDialogues;
    for (FTendencyDialogueData* Row : AllRows)
    {
        if (Row && Row->TendencyType == TendencyIndex)
        {
            FilteredDialogues.Add(Row->DialogueText);
        }
    }

    // 3. 필터링된 목록이 있다면 랜덤 선택
    if (FilteredDialogues.Num() > 0)
    {
        int32 RandIdx = FMath::RandRange(0, FilteredDialogues.Num() - 1);
        DialogueText = FilteredDialogues[RandIdx];

        // 4. UI 텍스트 업데이트
        if (InteractionUI)
        {
            if (UUserWidget* WidgetObj = InteractionUI->GetUserWidgetObject())
            {
                // 위젯 안에 "DialogueText"라는 이름의 텍스트 블록이 있다고 가정
                if (UTextBlock* TextComp = Cast<UTextBlock>(WidgetObj->GetWidgetFromName(TEXT("DialogueTxt"))))
                {
                    TextComp->SetText(FText::FromString(DialogueText));
                }
            }
        }
    }
}

void ATendencyActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (APawn* Pawn = Cast<APawn>(OtherActor))
    {
       if (Pawn->IsLocallyControlled())
       {
          // [신규] UI 켜기 전에 대사 랜덤 설정
          SetRandomDialogue();

          if (InteractionUI) InteractionUI->SetVisibility(true);
       }
    }  
}

void ATendencyActor::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (APawn* Pawn = Cast<APawn>(OtherActor))
    {
       if (Pawn->IsLocallyControlled())
       {
          if (InteractionUI) InteractionUI->SetVisibility(false);
       }
    }
}