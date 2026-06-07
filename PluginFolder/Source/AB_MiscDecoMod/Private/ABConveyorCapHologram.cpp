#include "ABConveyorCapHologram.h"
//#include "FGPipeSubsystem.h"

void AABConveyorCapHologram::ScrollRotate(int32 delta, int32 step) {
	if (mSnappedConnection != NULL) {
		//UE_LOG(LogTemp, Warning, TEXT("Le Flip?"));
		bFlipped = !bFlipped;
	} else {
		Super::ScrollRotate(delta, step);
	}
}

bool AABConveyorCapHologram::IsValidHitResult(const FHitResult& hitResult) const {
	// anythings valid so long as it isn't another cap, but since caps have building material swaps, check for something shared
	TObjectPtr<AFGBuildable> foundBuildable = Cast<AFGBuildable>(hitResult.GetActor());
	if (foundBuildable != NULL) {
		return !foundBuildable->mHologramClass->IsChildOf(GetClass());
	}
	return false;
}

bool AABConveyorCapHologram::CanNudgeHologram() const {
	return mSnappedConnection == NULL;
}

bool AABConveyorCapHologram::TrySnapToActor(const FHitResult& hitResult) {
	// have we hit anything good? reset if not
	TObjectPtr<AFGBuildable> foundBuildable = Cast<AFGBuildable>(hitResult.GetActor());
	if (foundBuildable == NULL) {
		mSnappedBuilding = NULL;
		mSnappedConnection = NULL;
		pIndex = -1;
		return false;
	}

	// is it new, save some effort if it isn't
	if (mSnappedBuilding != foundBuildable) {
		mSnappedBuilding = foundBuildable;
		mSnappedConnection = NULL;
		pIndex = -1;

		mSnappedBuilding->GetComponents<UFGConnectionComponent>(possibleConnections);

		if (!ValidatePotentialConnections()) { return false; }

		// if we got here we've got at least one valid connection so lets use it
		pIndex = 0;
	}

	// pick nearest for multiple candidates
	int len = possibleConnections.Num();
	//UE_LOG(LogTemp, Warning, TEXT("[X] [X] possible: %d"), len);
	if (len > 1) {
		FVector hitPos = hitResult.ImpactPoint;
		float bestDist = -1.0f;

		//UE_LOG(LogTemp, Warning, TEXT("[X] [X] hit x:%f, y:%f, z:%f"), hitPos.X, hitPos.Y, hitPos.Z);

		for (int i = 0; i < len; i++) {
			float dist = FVector::DistSquared(hitPos, possibleConnections[i]->GetComponentLocation());
			//UE_LOG(LogTemp, Warning, TEXT("[X] [X] dist[%d]: %f vs %f"), i, dist, (maxSubSnapDist * maxSubSnapDist));
			if ((dist < bestDist || bestDist < 0) && dist <= (maxSubSnapDist * maxSubSnapDist)) {
				//UE_LOG(LogTemp, Warning, TEXT("[X] [X] win!"));
				bestDist = dist;
				pIndex = i;
			}
		}
	}

	if (pIndex < 0 || pIndex >= len) { return false; }

	// move to our valid connector
	//UE_LOG(LogTemp, Warning, TEXT("[X] [X] pIndex: %d"), pIndex);

	if (mSnappedConnection != possibleConnections[pIndex]) { OnSnap(); }
	mSnappedConnection = possibleConnections[pIndex];

	SetActorTransform(mSnappedConnection->GetComponentTransform());

	bool flipable = false;
	EFactoryConnectionDirection beltType;
	EPipeConnectionType pipeType;
	if (validConnectionClass == EABCapType::CCT_Belt) {
		// check belt type
		beltType = Cast<UFGFactoryConnectionComponent>(mSnappedConnection)->GetDirection();
		flipable =
			beltType == EFactoryConnectionDirection::FCD_SNAP_ONLY;

	} else if (validConnectionClass == EABCapType::CCT_Pipe) {
		// check pipe type
		pipeType = Cast<UFGPipeConnectionComponentBase>(mSnappedConnection)->GetPipeConnectionType();
		flipable = pipeType == EPipeConnectionType::PCT_SNAP_ONLY;
	}

	// positions aren't perfectly consistent so, adjust
	for (int i = 0, keyCount = offsetMap.Num(); i < keyCount; i++) {
		//TSubclassOf<AFGBuildable> buildable = offsetMap[i].relevantBuildable;
		TSubclassOf<AFGBuildable> relevantClass = NULL;

		// Try using soft class path
		if (!IsValid(relevantClass)) {
			TObjectPtr<UClass> loadedBuildableClass = offsetMap[i].relevantBuildableSoftRef.TryLoadClass<AFGBuildable>();
			if (IsValid(loadedBuildableClass)) {
				relevantClass = loadedBuildableClass;
			}
		}

		// Try using found path
		if (relevantClass != NULL && mSnappedBuilding->GetClass()->IsChildOf(relevantClass)) {
			bool compatible = false;

			if (validConnectionClass == EABCapType::CCT_Belt) {
				// check belt type
				compatible =
					offsetMap[i].beltFilter == beltType ||
					offsetMap[i].beltFilter == EFactoryConnectionDirection::FCD_ANY ||
					beltType == EFactoryConnectionDirection::FCD_ANY;
				flipable =
					beltType == EFactoryConnectionDirection::FCD_SNAP_ONLY ||
					beltType == EFactoryConnectionDirection::FCD_ANY;

			} else if (validConnectionClass == EABCapType::CCT_Pipe) {
				// check pipe type
				compatible =
					offsetMap[i].pipeFilter == pipeType ||
					offsetMap[i].pipeFilter == EPipeConnectionType::PCT_ANY ||
					pipeType == EPipeConnectionType::PCT_ANY;
				flipable = pipeType == EPipeConnectionType::PCT_SNAP_ONLY;

			} else {
				UE_LOG(LogTemp, Error, TEXT("INVALID AB_CAP_TYPE USED"));
			}

			// if they set a name filter, test it too
			if (!offsetMap[i].nameFilter.IsNone()) {
				compatible &= offsetMap[i].nameFilter == mSnappedConnection->GetFName();
			}

			if (compatible) {
				//UE_LOG(LogTemp, Warning, TEXT("[X] [X] offset of %d using %d, %d"), pIndex, i, flipable);
				AddActorLocalOffset(offsetMap[i].offsetRequired);
				break;
			}
		}
	}

	if (flipable && bFlipped) {
		//UE_LOG(LogTemp, Warning, TEXT("[X] [X] offset FLIPPED"));
		AddActorLocalRotation(FRotator(0.0, 180.0, 0.0));
	}

	return true;
}

bool AABConveyorCapHologram::ValidatePotentialConnections() {
	int len = possibleConnections.Num();
	if (len < 1) { return false; }

	for (int i = 0; i < len; i++) {
		if (validConnectionClass == EABCapType::CCT_Belt) {
			TObjectPtr<UFGFactoryConnectionComponent> beltCnx = Cast<UFGFactoryConnectionComponent>(possibleConnections[i]);
			if (beltCnx != NULL && !beltCnx->IsConnected()) { continue; }
			// TODO: hide arrows and rings in hologram placement mode
			// TODO: hide arrows and rings in hologram placement mode
			// TODO: hide arrows and rings in hologram placement mode
			// TODO: hide arrows and rings in hologram placement mode

		} else if (validConnectionClass == EABCapType::CCT_Pipe) {
			TObjectPtr<UFGPipeConnectionComponentBase> pipeCnx = Cast<UFGPipeConnectionComponentBase>(possibleConnections[i]);
			if (pipeCnx != NULL && !pipeCnx->IsConnected()) { continue; }
			// TODO: make hypertubes not count
			// TODO: make hypertubes not count
			// TODO: make hypertubes not count
			// TODO: make hypertubes not count
			// TODO: hide arrows and rings in hologram placement mode
			// TODO: hide arrows and rings in hologram placement mode
			// TODO: hide arrows and rings in hologram placement mode
			// TODO: hide arrows and rings in hologram placement mode

		} else {
			UE_LOG(LogTemp, Error, TEXT("INVALID AB_CAP_TYPE USED"));
		}

		// if the connection was good we would of skipped ahead by now, so kill it
		possibleConnections.RemoveAt(i);
		i--; len--;
	}

	return len > 0;
}

void AABConveyorCapHologram::ConfigureComponents(class AFGBuildable* inBuildable) const {
	if (validConnectionClass == EABCapType::CCT_Belt) {
		TObjectPtr < UFGFactoryConnectionComponent> myCnx = inBuildable->GetComponentByClass<UFGFactoryConnectionComponent>();
		if (myCnx != NULL) {
			TObjectPtr < UFGFactoryConnectionComponent> theirCnx = Cast<UFGFactoryConnectionComponent>(mSnappedConnection);
			myCnx->SetConnection(theirCnx);
		}

	} else if (validConnectionClass == EABCapType::CCT_Pipe) {
		TObjectPtr < UFGPipeConnectionFactory> myCnx = inBuildable->GetComponentByClass<UFGPipeConnectionFactory>();
		if (myCnx != NULL) {
			TObjectPtr < UFGPipeConnectionComponentBase> theirCnx = Cast<UFGPipeConnectionComponentBase>(mSnappedConnection);
			myCnx->SetConnection(theirCnx);
		}

	} else {
		UE_LOG(LogTemp, Error, TEXT("INVALID AB_CAP_TYPE USED"));
	}

	Super::ConfigureComponents(inBuildable);
}