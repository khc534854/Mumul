// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Blueprint/UserWidget.h"
#include "ObjectAndClassFinder.generated.h"

/* ======================
	Widget Entry
====================== */
USTRUCT(BlueprintType)
struct FWidgetClass
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere)
	FName Key;

	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="/Script/UMG.UserWidget"))
	TSoftClassPtr<UUserWidget> WidgetClass;
};

/* ======================
	Actor Entry
====================== */
USTRUCT(BlueprintType)
struct FActorClass
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere)
	FName Key;

	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="/Script/Engine.Actor"))
	TSoftClassPtr<AActor> ActorClass;
};

UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Object And Class Finder"))
class MUMUL_API UObjectAndClassFinder : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/* ======================
		Widget BP
	====================== */
	UPROPERTY(Config, EditAnywhere, Category="Widget")
	TArray<FWidgetClass> WidgetBlueprintClasses;

	/* ======================
		Actor BP
	====================== */
	UPROPERTY(Config, EditAnywhere, Category="Actor")
	TArray<FActorClass> ActorBlueprintClasses;

	static const UObjectAndClassFinder* Get()
	{
		return GetDefault<UObjectAndClassFinder>();
	}

	/* ======================
		Widget Getter
	====================== */
	template<typename T>
	TSubclassOf<T> GetWidgetClass(FName Key) const
	{
		static_assert(TIsDerivedFrom<T, UUserWidget>::Value);

		for (const FWidgetClass& Entry : WidgetBlueprintClasses)
		{
			if (Entry.Key != Key)
				continue;

			UClass* Loaded = Entry.WidgetClass.LoadSynchronous();
			if (!Loaded)
				break;

			if (!Loaded->IsChildOf(T::StaticClass()))
				break;

			return Loaded;
		}

		LogNotFound(Key, TEXT("Widget"));
		return nullptr;
	}

	/* ======================
		Actor Getter
	====================== */
	template<typename T>
	TSubclassOf<T> GetActorClass(FName Key) const
	{
		static_assert(TIsDerivedFrom<T, AActor>::Value);

		for (const FActorClass& Entry : ActorBlueprintClasses)
		{
			if (Entry.Key != Key)
				continue;

			UClass* Loaded = Entry.ActorClass.LoadSynchronous();
			if (!Loaded)
			{
				LogNotFound(Key, TEXT("Actor"));
				return nullptr;
			}

			if (!Loaded->IsChildOf(T::StaticClass()))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[ObjectAndClassFinder] Actor type mismatch. Key=%s Loaded=%s Expected=%s"),
					*Key.ToString(),
					*Loaded->GetName(),
					*T::StaticClass()->GetName());
				return nullptr;
			}

			return Loaded;
		}

		LogNotFound(Key, TEXT("Actor"));
		return nullptr;
	}

private:
	void LogNotFound(FName Key, const TCHAR* Type) const
	{
		UE_LOG(LogTemp, Error,
			TEXT("[ObjectAndClassFinder] %s not found or invalid. Key=%s"),
			Type, *Key.ToString());
	}
};
