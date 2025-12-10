#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h" // FTableRowBase 사용을 위해 추가
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "FCustomItemData.generated.h"

// [아이템 정보를 담을 구조체]
USTRUCT(BlueprintType)
struct FCustomItemData : public FTableRowBase
{
	GENERATED_BODY()

	// 아이템 고유 번호 (DataTable Row Name과 동일하게 사용할 수 있습니다)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID;

	// 아이템 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemName;

	// 아이템 설명
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ItemDescription;

	// [핵심] 부착 소켓 위치 (캐릭터 스켈레톤의 소켓 이름)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AttachSocketName;

	// [핵심] 트랜스폼 (위치/회전/크기 오프셋)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTransform RelativeTransform = FTransform::Identity;

	// 스태틱 매쉬 (아이템의 실제 3D 모델)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> ItemStaticMesh;

	// 아이템 섬네일 (UI 표시용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> ItemThumbnail;
};