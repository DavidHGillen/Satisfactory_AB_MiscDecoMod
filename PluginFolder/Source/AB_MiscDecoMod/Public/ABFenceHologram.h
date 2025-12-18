#pragma once

#include "CoreMinimal.h"
#include "Hologram/FGFenceHologram.h"
#include "ABFenceHologram.generated.h"

/**
 * 
 */
UCLASS()
class AB_MISCDECOMOD_API AABFenceHologram : public AFGFenceHologram
{
	GENERATED_BODY()

public:
	// how far to position from a wall specifically
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hologram|Grounding")
	float wallOffset = 25.0f;

	// how far to look up and down from the impact point
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hologram|Grounding")
	float vertRange_Up = 151.0f;
	// how far to look up and down from the impact point
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hologram|Grounding")
	float vertRange_Dn = 251.0f;

	// how many rays to project to perform grounding
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hologram|Grounding")
	int iAccuracy = 6;

	// what to hit for Grounding
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hologram|Grounding")
	TEnumAsByte<ECollisionChannel> eChannel;

protected:
	//
	UPROPERTY(EditDefaultsOnly, Category = "Hologram|BuildMode")
	TSubclassOf<UFGHologramBuildModeDescriptor> mBuildModeGrounded;

public:
// Factory Game
	virtual void GetSupportedBuildModes_Implementation(TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& out_buildmodes) const override;
	virtual bool DoMultiStepPlacement(bool isInputFromARelease) override;
	virtual bool TrySnapToActor(const FHitResult& hitResult) override;
	virtual void SetHologramLocationAndRotation(const FHitResult& hitResult) override;

// Custom
	virtual bool SetGroundedLocationAndRotation(const FHitResult& hitResult);
};
