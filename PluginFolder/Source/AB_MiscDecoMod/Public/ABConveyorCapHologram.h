#pragma once

#include "CoreMinimal.h"

#include "Hologram/FGBuildableHologram.h"
#include "FGFactoryConnectionComponent.h"
#include "FGPipeConnectionFactory.h"

#include "ABConveyorCapHologram.generated.h"

UENUM(BlueprintType)
enum class EABCapType : uint8 {
	CCT_None UMETA(DisplayName = "- Please select -"),
	CCT_Belt UMETA(DisplayName = "Conveyor"),
	CCT_Pipe UMETA(DisplayName = "Pipes")
};

USTRUCT(BlueprintType)
struct FABOffsetData {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FSoftClassPath relevantBuildableSoftRef;

	UPROPERTY(EditAnywhere)
	EABCapType connectionType;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "connectionType==EABCapType::CCT_Pipe"))
	EPipeConnectionType pipeFilter;

	UPROPERTY(EditAnywhere, meta = (EditCondition = "connectionType==EABCapType::CCT_Belt"))
	EFactoryConnectionDirection beltFilter;

	UPROPERTY(EditAnywhere)
	FName nameFilter;

	UPROPERTY(EditAnywhere)
	FVector offsetRequired;
};

/**
 * 
 */
UCLASS(BlueprintType)
class AB_MISCDECOMOD_API AABConveyorCapHologram : public AFGBuildableHologram
{
	GENERATED_BODY()

public:
	// How to adjust the final position when snapping to ports, relative to that port, for the given buildables
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hologram|Cap")
	TArray<FABOffsetData> offsetMap;

	// What classes of connectors to accept or filter out
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hologram|Cap")
	EABCapType validConnectionClass;

	// What distance away from the valid connector should we try to connect
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hologram|Cap")
	float maxSubSnapDist;

protected:
	// The connection on the building we're snapped to
	UFGConnectionComponent* mSnappedConnection;

	// The connection on the building we're snapped to
	TArray<UFGConnectionComponent*> possibleConnections;

	// The index of the potential connection we're at
	int pIndex;

	// Is it flipped about the snap point it's on
	bool bFlipped;

public:
	// thin the possible connections array to only valid results, return if there's anything left
	virtual bool ValidatePotentialConnections();

	// FactoryGame:
	//virtual void GetSupportedBuildModes_Implementation(TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& out_buildmodes) const override;
	//virtual void OnBuildModeChanged(TSubclassOf<UFGHologramBuildModeDescriptor> buildMode) override;
	virtual void ScrollRotate(int32 delta, int32 step);
	virtual bool IsValidHitResult(const FHitResult& hitResult) const override;
	virtual bool CanNudgeHologram() const override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;

protected:
	virtual void ConfigureComponents(class AFGBuildable* inBuildable) const;
};
