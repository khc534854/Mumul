// Fill out your copyright notice in the Description page of Project Settings.


#include "Yeomin/UI/GroupIconUI.h"
#include "Yeomin/UI/ChatBlockUI.h"
#include "Components/Button.h"
#include "Yeomin/UI/GroupChatUI.h"

void UGroupIconUI::NativeConstruct()
{
	Super::NativeConstruct();
	
	GroupChatUI = CreateWidget<UChatBlockUI>(GetWorld(), ChatBlockUIClass);
	GroupIconBtn->OnPressed.AddDynamic(this, &UGroupIconUI::DisplayGroupChat);
}



void UGroupIconUI::DisplayGroupChat()
{
	ParentUI->RemoveChatBlock();
	ParentUI->AddChatBlock(GroupChatUI);
}

void UGroupIconUI::InitParentUI(UGroupChatUI* Parent)
{
	ParentUI = Parent;
}
