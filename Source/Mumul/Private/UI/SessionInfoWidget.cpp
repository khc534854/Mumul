// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/SessionInfoWidget.h"
#include "Base/MumulGameInstance.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void USessionInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();
	btn_Join->OnClicked.AddDynamic(this,&USessionInfoWidget::OnClickJoin);
}

void USessionInfoWidget::SetSessionInfo(int32 idx, FString sessionName)
{
	sessionidx = idx;
	textSessionName->SetText(FText::FromString(sessionName));
	
}

void USessionInfoWidget::OnClickJoin()
{
	UMumulGameInstance* gi = Cast<UMumulGameInstance>(GetWorld()->GetGameInstance());
	gi ->JoinGameSession(sessionidx);
}
