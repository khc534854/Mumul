// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Blueprint/UserWidget.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundMix.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#endif
#include "ObjectAndClassFinder.generated.h"

USTRUCT(BlueprintType)
struct FWidgetClass
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere)
	FName Key;

	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="/Script/UMG.UserWidget"))
	TSoftClassPtr<UUserWidget> WidgetClass;
};

USTRUCT(BlueprintType)
struct FActorClass
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere)
	FName Key;

	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="/Script/Engine.Actor"))
	TSoftClassPtr<AActor> ActorClass;
};

USTRUCT(BlueprintType)
struct FSoundAsset
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere)
	FName Key;

	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="/Script/Engine.SoundBase"))
	TSoftObjectPtr<USoundBase> Sound;
};

USTRUCT(BlueprintType)
struct FSoundMixAsset
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere)
	FName Key;

	UPROPERTY(Config, EditAnywhere, meta = (AllowedClasses="/Script/Engine.SoundMix"))
	TSoftObjectPtr<USoundMix> SoundMix;
};

UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Object And Class Finder"))
class MUMUL_API UObjectAndClassFinder : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="Widget")
	TArray<FWidgetClass> WidgetBlueprintClasses;

	UPROPERTY(Config, EditAnywhere, Category="Actor")
	TArray<FActorClass> ActorBlueprintClasses;

	UPROPERTY(Config, EditAnywhere, Category="Sound")
	TArray<FSoundAsset> SoundAssets;

	UPROPERTY(Config, EditAnywhere, Category="Sound")
	TArray<FSoundMixAsset> SoundMixAssets;

	static const UObjectAndClassFinder* Get()
	{
		return GetDefault<UObjectAndClassFinder>();
	}

	template <typename T>
	TSubclassOf<T> GetWidgetClass(FName Key) const
	{
		static_assert(TIsDerivedFrom<T, UUserWidget>::Value, "T must derive from UUserWidget");

		for (const FWidgetClass& Entry : WidgetBlueprintClasses)
		{
			if (Entry.Key != Key)
				continue;

			// 방어 1) null이면 즉시 실패(continue가 아니라 return: 같은 Key가 여러개면 첫 매치가 기준이므로)
			if (Entry.WidgetClass.IsNull())
			{
				LogNotFound(Key, TEXT("Widget (null soft class)"));
				return nullptr;
			}

			// 방어 2) 동기 로드 실패 시 break 하지 말고 "정상 실패 처리"
			UClass* Loaded = Entry.WidgetClass.LoadSynchronous();
			if (!IsValid(Loaded))
			{
				LogNotFound(Key, TEXT("Widget (failed to load class)"));
				return nullptr;
			}

			// 방어 3) 타입 불일치도 break 하지 말고 명확히 로그 + 실패 처리
			if (!Loaded->IsChildOf(T::StaticClass()))
			{
				UE_LOG(LogTemp, Error,
					TEXT("[ObjectAndClassFinder] Widget type mismatch. Key=%s Loaded=%s Expected=%s"),
					*Key.ToString(),
					*Loaded->GetName(),
					*T::StaticClass()->GetName());
				return nullptr;
			}

			return Loaded;
		}

		LogNotFound(Key, TEXT("Widget"));
		return nullptr;
	}

	template <typename T>
	TSubclassOf<T> GetActorClass(FName Key) const
	{
		static_assert(TIsDerivedFrom<T, AActor>::Value, "T must derive from AActor");

		for (const FActorClass& Entry : ActorBlueprintClasses)
		{
			if (Entry.Key != Key)
				continue;

			if (Entry.ActorClass.IsNull())
			{
				LogNotFound(Key, TEXT("Actor (null soft class)"));
				return nullptr;
			}

			UClass* Loaded = Entry.ActorClass.LoadSynchronous();
			if (!IsValid(Loaded))
			{
				LogNotFound(Key, TEXT("Actor (failed to load class)"));
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

	USoundBase* GetSound(FName Key) const
	{
		for (const FSoundAsset& Entry : SoundAssets)
		{
			if (Entry.Key != Key)
				continue;

			if (Entry.Sound.IsNull())
			{
				LogNotFound(Key, TEXT("Sound (null soft object)"));
				return nullptr;
			}

			USoundBase* Loaded = Entry.Sound.LoadSynchronous();
			if (!IsValid(Loaded))
			{
				LogNotFound(Key, TEXT("Sound (failed to load object)"));
				return nullptr;
			}

			return Loaded;
		}

		LogNotFound(Key, TEXT("Sound"));
		return nullptr;
	}

	USoundMix* GetSoundMix(FName Key) const
	{
		for (const FSoundMixAsset& Entry : SoundMixAssets)
		{
			if (Entry.Key != Key)
				continue;

			if (Entry.SoundMix.IsNull())
			{
				LogNotFound(Key, TEXT("SoundMix (null soft object)"));
				return nullptr;
			}

			USoundMix* Loaded = Entry.SoundMix.LoadSynchronous();
			if (!IsValid(Loaded))
			{
				LogNotFound(Key, TEXT("SoundMix (failed to load object)"));
				return nullptr;
			}

			return Loaded;
		}

		LogNotFound(Key, TEXT("SoundMix"));
		return nullptr;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);

		// 에디터 방어 핵심:
		// "설정창에서 클릭했을 때 터지는" 원인 1순위가
		// 깨진 Soft 경로(삭제/이동/리다이렉터 꼬임)인데, 이걸 미리 걸러서 null로 비웁니다.
		IAssetRegistry* AR = nullptr;
		if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetRegistry")))
		{
			AR = &FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		}

		for (FWidgetClass& Entry : WidgetBlueprintClasses)
		{
			if (Entry.WidgetClass.IsNull())
				continue;

			const FSoftObjectPath ClassPath = Entry.WidgetClass.ToSoftObjectPath();

			// 경로 문자열 자체가 이상하면 바로 제거
			if (!ClassPath.IsValid())
			{
				UE_LOG(LogTemp, Warning,
					TEXT("[ObjectAndClassFinder] Clearing invalid WidgetClass path. Key=%s Path=%s"),
					*Entry.Key.ToString(), *ClassPath.ToString());
				Entry.WidgetClass = nullptr;
				continue;
			}

			// 핵심: TSoftClassPtr은 보통 ..._C(GeneratedClass)를 가리킴.
			// AssetRegistry는 대개 블루프린트 에셋(...WBP_AskOXQuiz.WBP_AskOXQuiz)을 인덱싱하므로
			// "에셋 경로"로 조회해야 오탐이 줄어듭니다.
			if (AR)
			{
				const FString AssetPathString = ClassPath.GetAssetPathString(); // "/Game/.../WBP_AskOXQuiz.WBP_AskOXQuiz"
				const FAssetData Data = AR->GetAssetByObjectPath(FSoftObjectPath(AssetPathString));

				if (!Data.IsValid())
				{
					UE_LOG(LogTemp, Warning,
						TEXT("[ObjectAndClassFinder] WidgetClass asset not found in AssetRegistry. Key=%s ClassPath=%s AssetPath=%s (will not auto-clear)"),
						*Entry.Key.ToString(),
						*ClassPath.ToString(),
						*AssetPathString);

					// 주의: 여기서 auto-clear까지 해버리면 오탐/타이밍 문제로 세팅을 날릴 수 있음.
					// 정말 삭제된 것만 확신할 수 있을 때만 null로 비우는 걸 추천합니다.
					// Entry.WidgetClass = nullptr;
				}
			}
		}
	}
#endif

private:
	void LogNotFound(FName Key, const TCHAR* Type) const
	{
		UE_LOG(LogTemp, Error,
			TEXT("[ObjectAndClassFinder] %s not found or invalid. Key=%s"),
			Type, *Key.ToString());
	}
};