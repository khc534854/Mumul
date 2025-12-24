#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FTendencyDialogueData.generated.h"


USTRUCT(BlueprintType)
struct FTendencyDialogueData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 TendencyType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString DialogueText;
};
