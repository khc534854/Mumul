#pragma once

#include "CoreMinimal.h"
#include "FSaveData.generated.h"

USTRUCT(BlueprintType)
struct FSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	FString OwnerUserID;

	UPROPERTY()
	FTransform Transform;
	
};