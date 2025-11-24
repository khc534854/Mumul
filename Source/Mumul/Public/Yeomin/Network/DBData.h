#pragma once

#include "CoreMinimal.h"
#include "DBData.generated.h"

USTRUCT(BlueprintType)
struct FDBData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString UserID;

	UPROPERTY(BlueprintReadWrite)
	int32 Score;
};
