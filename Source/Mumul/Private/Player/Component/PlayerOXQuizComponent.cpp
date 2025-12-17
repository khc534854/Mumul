// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/PlayerOXQuizComponent.h"

#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Data/ObjectAndClassFinder.h"
#include "GameFramework/GameStateBase.h"
#include "Object/CloudActor.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "UI/OXQuiz/OXQuizUI.h"


UPlayerOXQuizComponent::UPlayerOXQuizComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}


void UPlayerOXQuizComponent::BeginPlay()
{
	Super::BeginPlay();

	owner = Cast<ACuteAlienController>(GetOwner());
	if (owner)
		player = Cast<ACuteAlienPlayer>(owner->GetPawn());

	if (owner && owner->IsLocalController())
	{
		if (TSubclassOf<UOXQuizUI> OXQuizUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<
			UOXQuizUI>("WBP_OXQuiz"))
		{
			OXQuizUI = CreateWidget<UOXQuizUI>(owner, OXQuizUIClass);
			if (OXQuizUI)
			{
				OXQuizUI->AddToViewport();
				OXQuizUI->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}


void UPlayerOXQuizComponent::Client_DisplayReady_Implementation()
{
	if (OXQuizUI)
	{
		OXQuizUI->OXQuizWS->SetActiveWidgetIndex(3);
		OXQuizUI->SetVisibility(ESlateVisibility::Visible);
	}
}


void UPlayerOXQuizComponent::Client_HideOXQuizUI_Implementation()
{
	OXQuizUI->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerOXQuizComponent::UpdateCountdown()
{
	const float ServerNow = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	const int32 Remain = FMath::CeilToInt(CountdownEndTime - ServerNow);

	if (Remain <= 0)
	{
		OXQuizUI->ReadyText->SetText(FText::FromString(TEXT("출발!")));
		GetWorld()->GetTimerManager().ClearTimer(ReadyCountdownTimer);
		return;
	}

	OXQuizUI->ReadyText->SetText(FText::AsNumber(Remain));
}

void UPlayerOXQuizComponent::Client_StartReadyCountdown_Implementation(int32 Time)
{
	const float ServerNow = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	CountdownEndTime = ServerNow + Time;

	GetWorld()->GetTimerManager().ClearTimer(ReadyCountdownTimer);
	GetWorld()->GetTimerManager().SetTimer(
		ReadyCountdownTimer,
		this,
		&UPlayerOXQuizComponent::UpdateCountdown,
		0.1f,
		true
	);
}

void UPlayerOXQuizComponent::Client_DisplayQuestion_Implementation(const int32& QuestionIdx, const FString& NewQuestion,
                                                                   const int32& QuestionTime)
{
	if (OXQuizUI)
	{
		OXQuizUI->SwitchQuizState(true);
		OXQuizUI->SetVisibility(ESlateVisibility::HitTestInvisible);
		OXQuizUI->SetQuizQuestion(QuestionIdx, NewQuestion);
		OXQuizUI->StartQuestionTimer(QuestionTime);
	}
}

void UPlayerOXQuizComponent::Client_DisplayAnswer_Implementation(bool AnswerResult, bool NewAnswer,
                                                                 const FString& NewCommentary, const int32& AnswerTime)
{
	bool CheckAnswer = true;
	if (AnswerResult != NewAnswer)
	{
		CheckAnswer = false;
		Server_SpawnCloud(player);
	}

	OXQuizUI->SetQuizAnswer(CheckAnswer, NewAnswer, NewCommentary);
	OXQuizUI->StartAnswerTimer(AnswerTime);
}

void UPlayerOXQuizComponent::Client_DisplayResult_Implementation(const int32& QuestionIdx, bool AnswerResult,
                                                                 const FString& QuestionText,
                                                                 bool AnswerText, const FString& CommentaryText)
{
	bool CheckAnswer = false;
	if (AnswerResult == AnswerText)
	{
		CheckAnswer = true;
	}

	OXQuizUI->SwitchQuizState(false);
	OXQuizUI->SetVisibility(ESlateVisibility::Visible);
	OXQuizUI->SetQuizResult(QuestionIdx, CheckAnswer, QuestionText, AnswerText, CommentaryText);
}

void UPlayerOXQuizComponent::Server_SpawnCloud_Implementation(class ACuteAlienPlayer* OwnerPlayer)
{
	TSubclassOf<ACloudActor> CloudClass = UObjectAndClassFinder::Get()->GetActorClass<ACloudActor>("BP_Cloud");
	FVector SpawnLocation = OwnerPlayer->GetActorLocation() + FVector(0.f, 0.f, 200.f);
	FRotator SpawnRotation = OwnerPlayer->GetActorRotation();
	ACloudActor* Cloud = GetWorld()->SpawnActor<ACloudActor>(CloudClass, SpawnLocation, SpawnRotation);
	Cloud->InitOwnerPlayer(OwnerPlayer);
}
