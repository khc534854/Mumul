#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "FHousingItemData.generated.h"


USTRUCT(BlueprintType)
struct FHousingItemData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;

	// 아이템 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemDescription;

	// 스태틱 매쉬 (아이템의 실제 3D 모델)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> ItemStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> ItemThumbnail;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsPlaceable = true;
};
