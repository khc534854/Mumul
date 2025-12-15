// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Blueprint/UserWidget.h"
#include "ObjectAndClassFinder.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FWidgetClass
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere)
	FName Key;

	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses = "/Script/UMG.UserWidget"))
	TSoftClassPtr<UUserWidget> WidgetClass;
};

UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Object And Class Finder"))
class MUMUL_API UObjectAndClassFinder : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="WBP")
	TArray<FWidgetClass> WidgetBlueprintClasses;

	static const UObjectAndClassFinder* Get()
	{
		return GetDefault<UObjectAndClassFinder>();
	}

	template<typename T>
TSubclassOf<T> GetWidgetClass(FName Key) const
	{
		static_assert(TIsDerivedFrom<T, UUserWidget>::Value);

		for (const FWidgetClass& Entry : WidgetBlueprintClasses)
		{
			if (Entry.Key != Key)
				continue;

			UClass* LoadedClass = Entry.WidgetClass.LoadSynchronous();
			if (!LoadedClass)
			{
				UE_LOG(LogTemp, Error,
					TEXT("[WidgetFinder] Load failed. Key=%s"),
					*Key.ToString());
				return nullptr;
			}

			if (!LoadedClass->IsChildOf(T::StaticClass()))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[WidgetFinder] Type mismatch. Key=%s Loaded=%s Expected=%s"),
					*Key.ToString(),
					*LoadedClass->GetName(),
					*T::StaticClass()->GetName());
				return nullptr;
			}

			return LoadedClass;
		}

		UE_LOG(LogTemp, Error,
			TEXT("[WidgetFinder] Key not found. Key=%s"),
			*Key.ToString());

		return nullptr;
	}
};
