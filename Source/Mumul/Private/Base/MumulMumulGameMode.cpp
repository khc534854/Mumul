// Fill out your copyright notice in the Description page of Project Settings.


#include "Base/MumulMumulGameMode.h"

#include "JsonObjectConverter.h"
#include "Base/MumulGameState.h"
#include "Components/BoxComponent.h"
#include "Player/MumulPlayerState.h"
#include "Save/MapDataSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Network/HttpNetworkSubsystem.h"
#include "Network/NetworkStructs.h"
#include "Object/OXQuizActor.h"
#include "Object/Tent/TentActor.h"
#include "Player/CuteAlienController.h"

AMumulMumulGameMode::AMumulMumulGameMode()
{
	static ConstructorHelpers::FClassFinder<ATentActor> TentFinder(
		TEXT("/Game/Yeomin/Actors/Tent/BP_Tent.BP_Tent_C"));
	if (TentFinder.Succeeded())
	{
		TentClass = TentFinder.Class;
	}
}

void AMumulMumulGameMode::BeginPlay()
{
	Super::BeginPlay();

	GS = GetGameState<AMumulGameState>();

	for (int i = 0; i < PoolSize; i++)
	{
		ATentActor* Tent = GetWorld()->SpawnActor<ATentActor>(TentClass);

		if (Tent == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("Tent Spawn FAILED at %d"), i);
			continue;
		}

		Tent->Deactivate();
		TentPool.Add(Tent);
	}

	FString SlotName = TEXT("IslandMapSave");
	if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UMapDataSaveGame* LoadInst = Cast<UMapDataSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
		if (LoadInst)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LoadGame] Restoring %d Tents..."), LoadInst->SavedTents.Num());

			for (const FTentSaveData& Data : LoadInst->SavedTents)
			{
				// 저장된 정보로 텐트 스폰 (기존 SpawnTent 함수 재활용)
				// 주의: SpawnTent 함수 내부에서 또 저장을 호출하지 않도록 플래그 처리가 필요할 수 있음.
				// 여기서는 단순히 '복구'만 하므로 SpawnTent를 직접 부르거나, 
				// SpawnTent 로직을 분리하는 것이 좋습니다.

				// 일단 직접 배치 로직 (SpawnTent 함수 활용)
				SpawnTent(Data.Transform, Data.OwnerUserIndex, false); // false: 저장하지 마라 (불러오는 중이니까)
			}
		}
	}

	UHttpNetworkSubsystem* HttpSystem = GetGameInstance()->GetSubsystem<UHttpNetworkSubsystem>();
	if (HttpSystem)
	{
		HttpSystem->OnLearningQuizResponse.AddDynamic(this, &AMumulMumulGameMode::OnServerLearningQuizResponse);
	}
	
}

void AMumulMumulGameMode::Logout(AController* Exiting)
{
	SaveUserData(Exiting);
	Super::Logout(Exiting);
}

void AMumulMumulGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC)
		{
			SaveUserData(PC);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AMumulMumulGameMode::SaveUserData(AController* Controller)
{
	if (!Controller) return;

	AMumulPlayerState* PS = Controller->GetPlayerState<AMumulPlayerState>();
	APawn* Pawn = Controller->GetPawn();

	// 폰이 없으면 저장할 위치도 없으므로 패스 (혹은 PlayerState에 저장된 마지막 위치 사용)
	if (PS && Pawn)
	{
		int32 UserIndex = PS->PS_UserIndex;

		// 유효한 유저라면 저장
		if (UserIndex > 0)
		{
			if (GS)
			{
				// 위치 저장
				GS->Multicast_SavePlayerLocation(UserIndex, Pawn->GetActorTransform());
				UE_LOG(LogTemp, Warning, TEXT("[GameMode] Saved User %d Location: %s"), UserIndex,
				       *Pawn->GetActorTransform().GetLocation().ToString());
			}
		}
	}
}

void AMumulMumulGameMode::SpawnTent(const FTransform& SpawnTransform, int32 UserIndex, bool bSaveToDisk)
{
	bool bTentProcessed = false; // 텐트 처리가 완료되었는지 확인하는 플래그
	ATentActor* TargetTent = nullptr;

	// 1. 이미 해당 유저가 활성화한 텐트가 있는지 확인 (이동 로직)
	for (const TPair<TObjectPtr<ATentActor>, int32>& PoolElem : TentPool)
	{
		if (PoolElem.Value == UserIndex && PoolElem.Key->bIsActive)
		{
			TargetTent = PoolElem.Key;
			TargetTent->ChangeTransform(SpawnTransform);
			TargetTent->Mulicast_OnScaleAnimation();

			bTentProcessed = true;
			UE_LOG(LogTemp, Log, TEXT("[GameMode] Found existing tent for User %d. Moving..."), UserIndex);
			break; // 리턴 대신 루프 탈출
		}
	}

	// 2. 처리가 안 됐다면(내 텐트가 없다면), 빈 텐트 찾기 (재사용 로직)
	if (!bTentProcessed)
	{
		for (TPair<TObjectPtr<ATentActor>, int32>& PoolElem : TentPool)
		{
			if (!PoolElem.Key->bIsActive)
			{
				TargetTent = PoolElem.Key;
				TargetTent->SetOwnerUserIndex(UserIndex);
				TargetTent->Activate(SpawnTransform);
				TargetTent->Mulicast_OnScaleAnimation();
				PoolElem.Value = UserIndex; // 주인 업데이트

				bTentProcessed = true;
				UE_LOG(LogTemp, Log, TEXT("[GameMode] Recycled inactive tent for User %d."), UserIndex);
				break; // 리턴 대신 루프 탈출
			}
		}
	}

	// 3. 여전히 처리가 안 됐다면(빈 텐트도 없다면), 새로 생성 (확장 로직)
	if (!bTentProcessed)
	{
		TargetTent = GetWorld()->SpawnActor<ATentActor>(TentClass);
		if (TargetTent)
		{
			TargetTent->SetOwnerUserIndex(UserIndex);
			TargetTent->Activate(SpawnTransform);
			TentPool.Add(TargetTent, UserIndex);

			bTentProcessed = true;
			UE_LOG(LogTemp, Log, TEXT("[GameMode] Spawned new tent for User %d."), UserIndex);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Tent Spawn FAILED to ADD"));
		}
	}

	// 4. [핵심] 텐트 처리가 성공했고, 저장 옵션이 켜져있다면 저장 실행
	// (리턴을 없앴기 때문에 이제 이 코드가 실행됩니다)
	if (bTentProcessed && bSaveToDisk)
	{
		if (GS)
		{
			GS->Multicast_SaveTentData(UserIndex, SpawnTransform);
			UE_LOG(LogTemp, Log, TEXT("[GameMode] Requesting Multicast Save for User %d..."), UserIndex);
		}
	}
}

void AMumulMumulGameMode::RegisterQuizActor(class AOXQuizActor* InActor)
{
	OXQuizActor = InActor;
}

void AMumulMumulGameMode::OnServerLearningQuizResponse(bool bSuccess, FString Message)
{
	if (bSuccess)
	{
		// 1. JSON 파싱 (Message에는 JSON 원본이 들어있음)
		LearningQuiz = FLearningQuizResponse();
		if (FJsonObjectConverter::JsonObjectStringToUStruct(Message, &LearningQuiz, 0, 0))
		{
			// JSON Parsing LOG
			UE_LOG(LogTemp, Log, TEXT("isLearningQuestion: %s"),
			       LearningQuiz.isLearningQuestion ? TEXT("true") : TEXT("false"));
			UE_LOG(LogTemp, Log, TEXT("grade: %s"), *LearningQuiz.grade);

			UE_LOG(LogTemp, Log, TEXT("Quiz Count: %d"), LearningQuiz.quiz.Num());
			for (int32 i = 0; i < LearningQuiz.quiz.Num(); i++)
			{
				const FQuizContent& Content = LearningQuiz.quiz[i];
				UE_LOG(LogTemp, Log, TEXT("---- Quiz %d ----"), i + 1);
				UE_LOG(LogTemp, Log, TEXT("id: %d"), Content.id);
				UE_LOG(LogTemp, Log, TEXT("type: %s"), *Content.type);
				UE_LOG(LogTemp, Log, TEXT("question: %s"), *Content.question);
				UE_LOG(LogTemp, Log, TEXT("answer: %s"), *Content.answer);
				UE_LOG(LogTemp, Log, TEXT("explanation: %s"), *Content.explanation);
			}

			StartLearningQuiz();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("LearningQuiz 파싱 실패"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LearningQuiz Response 실패 : %s"), *Message);
	}
}

void AMumulMumulGameMode::StartLearningQuiz()
{
	// Init Quiz
	CurrentQuizIdx = 0;
	MaxQuizCount = LearningQuiz.quiz.Num();
	StartQuestionPhase();
}

void AMumulMumulGameMode::StartQuestionPhase()
{
	QuizPhase = EQuizPhase::Question;

	UE_LOG(LogTemp, Warning, TEXT("문제 %d 표시"), CurrentQuizIdx);
	// 문제 표시
	for (TPair<TObjectPtr<ACuteAlienController>, TArray<bool>>& Elem : ParticipatingPlayers)
	{
		Elem.Key->Client_DisplayQuestion(LearningQuiz.quiz[CurrentQuizIdx].question, QuestionTime);
	}


	GetWorld()->GetTimerManager().SetTimer(
		QuizTimer, this, &AMumulMumulGameMode::EnterNextStep, 2.f, false);
}

void AMumulMumulGameMode::EnterNextStep()
{
	if (QuizPhase == EQuizPhase::Question)
	{
		StartAnswerPhase();
		return;
	}

	if (QuizPhase == EQuizPhase::Answer)
	{
		if (CurrentQuizIdx == MaxQuizCount - 1)
		{
			ShowResult();
			return;
		}
		
		CurrentQuizIdx++;
		StartQuestionPhase();
	}
}

void AMumulMumulGameMode::StartAnswerPhase()
{
	// 정답 판정
	OXQuizActor->JudgePlayerAnswers();
	
	QuizPhase = EQuizPhase::Answer;
	
	UE_LOG(LogTemp, Warning, TEXT("해설 %d 표시"), CurrentQuizIdx);

	bool Answer = false;
	if (LearningQuiz.quiz[CurrentQuizIdx].answer == "O")
	{
		Answer = true;
	}
	else if (LearningQuiz.quiz[CurrentQuizIdx].answer == "X")
	{
		Answer = false;
	}

	// 해설 표시
	for (TPair<TObjectPtr<ACuteAlienController>, TArray<bool>>& Elem : ParticipatingPlayers)
	{
		Elem.Key->Client_DisplayAnswer(Elem.Value[CurrentQuizIdx], Answer, LearningQuiz.quiz[CurrentQuizIdx].explanation, AnswerTime);
	}

	GetWorld()->GetTimerManager().SetTimer(
		QuizTimer, this, &AMumulMumulGameMode::EnterNextStep, 1.f, false);
}

void AMumulMumulGameMode::ShowResult()
{
	// 정답 판정
	OXQuizActor->JudgePlayerAnswers();
	
	// 전체 정답 표기
	for (TPair<TObjectPtr<ACuteAlienController>, TArray<bool>>& Elem : ParticipatingPlayers)
	{
		for (int32 QuestionIdx = 0; QuestionIdx < MaxQuizCount; QuestionIdx++)
		{
			bool Answer = false;
			if (LearningQuiz.quiz[QuestionIdx].answer == "O")
			{
				Answer = true;
			}
			else if (LearningQuiz.quiz[QuestionIdx].answer == "X")
			{
				Answer = false;
			}
			Elem.Key->Client_DisplayResult(Elem.Value[QuestionIdx], LearningQuiz.quiz[QuestionIdx].question, Answer, LearningQuiz.quiz[QuestionIdx].explanation);
		}
	}
	
	OXQuizActor->FrontBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OXQuizActor->BackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OXQuizActor->LeftBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OXQuizActor->RightBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
