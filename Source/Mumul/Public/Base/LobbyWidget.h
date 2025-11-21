// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * 
 */
UCLASS()
class MUMUL_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	UPROPERTY()
	class UMumulGameInstance* gi;

	UPROPERTY(meta=(BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;

	UPROPERTY(meta=(BindWidget))
	class UButton* btn_goCreate;

	UPROPERTY(meta=(BindWidget))
	class UButton* btn_goFind;
	
	UPROPERTY(meta=(BindWidget))
	class UScrollBox* scrollSessionList;
	
	UPROPERTY(meta=(BindWidget))
	class UButton* btn_find;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* textFind;

	UPROPERTY(meta=(BindWidget))
	class UButton* btn_BackFromFind;

	UPROPERTY(meta=(BindWidget))
	class UButton* btn_BackFromCreate;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class USessionInfoWidget> sessionInfoWidget;
	
	
public:
	UFUNCTION()
	void OnClickGoCreate();
	UFUNCTION()
	void OnClickGoFind();
	UPROPERTY(meta=(BindWidget))
	class UEditableTextBox* editSessionName;
	//인원수 Slider
	UPROPERTY(meta=(BindWidget))
	class USlider* sliderPlayerCount;
	//인원수 텍스트
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* textPlayerCount;
	//크리에이트 버튼
	UPROPERTY(meta=(BindWidget))
	class UButton* btn_Create;
	UFUNCTION()
	void OnClickCreate();
	UFUNCTION()
	void OnValudeChangedSessionName(const FText& text);
	UFUNCTION()
	void OnValudeChangedPlayerCount(float value);
	UFUNCTION()
	void OnClickFind();
	//세션정보를 받아 SessionInfoWidget을 만드는 함수
	//(NetGameInstance의 onFindComplete 델리게이트에 등록할 함수
	UFUNCTION()
	void OnFindComplete(int32 idx, FString sessionName);
	UFUNCTION()
	void OnClickBack();

	UFUNCTION()
	void RefreshSessionList(bool bWasSuccessful);
};
