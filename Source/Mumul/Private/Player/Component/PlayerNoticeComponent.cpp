// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Component/PlayerNoticeComponent.h"

#include "Data/ObjectAndClassFinder.h"
#include "Player/CuteAlienController.h"
#include "Player/CuteAlienPlayer.h"
#include "UI/NoticeUI/NoticeUI.h"


UPlayerNoticeComponent::UPlayerNoticeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


void UPlayerNoticeComponent::BeginPlay()
{
	Super::BeginPlay();
	
	owner = Cast<ACuteAlienController>(GetOwner());
	if (owner)
		player = Cast<ACuteAlienPlayer>(owner->GetPawn());
	
	if (owner && owner->IsLocalController())
	{
		if (TSubclassOf<UNoticeUI> NoticeUIClass = UObjectAndClassFinder::Get()->GetWidgetClass<UNoticeUI>("WBP_Notice"))
		{
			NoticeUI = CreateWidget<UNoticeUI>(owner, NoticeUIClass);
			if (NoticeUI)
			{
				NoticeUI->AddToViewport();
				NoticeUI->SetVisibility(ESlateVisibility::Visible);
			}
		}
	}
}


void UPlayerNoticeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

