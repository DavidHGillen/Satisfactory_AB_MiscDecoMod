#pragma once

#include "CoreMinimal.h"
#include "FGFactoryConnectionComponent.h"
#include "ABFactoryDeadEndComponent.generated.h"

/**
 * 
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class AB_MISCDECOMOD_API UABFactoryDeadEndComponent : public UFGFactoryConnectionComponent
{
	GENERATED_BODY()

	class AFGBuildable* myBuildable;
	
public:
	void BeginPlay() {
		// If this connection never asks for anything, it never has anything. So it's a safe no operation.
		// Except for intelligent buildings that try to query the belt like smart splitters.
		// Lying that it has no outer building breaks nothing else but inspections that would only cause issues**.
		myBuildable = mOuterBuildable;
		mOuterBuildable = nullptr;
	}

	void OnUnregister() {
		// **make sure cleanup does the right thing, but tell the truth so it doesn't crash
		mOuterBuildable = myBuildable;
		Super::OnUnregister();
	}
};
