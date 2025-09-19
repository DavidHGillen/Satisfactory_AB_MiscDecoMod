#include "ABConveyorCapHologram.h"
//#include "FGPipeSubsystem.h"

bool AABConveyorCapHologram::IsValidHitResult(const FHitResult& hitResult) const {
	if (Cast<AFGBuildable>(hitResult.GetActor()) != NULL) {
		return true;
	}
	return false;
}

bool AABConveyorCapHologram::CanNudgeHologram() const {
	return mSnappedConnection == NULL;
}

bool AABConveyorCapHologram::TrySnapToActor(const FHitResult& hitResult) {
	// have we hit anything good? reset if not
	AFGBuildable* foundBuildable = Cast<AFGBuildable>(hitResult.GetActor());
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

	// positions aren't perfectly consistent so, adjust
	for (int i = 0, keyCount = offsetMap.Num(); i < keyCount; i++) {
		if (mSnappedBuilding->GetClass()->IsChildOf(offsetMap[i].relevantBuildable)) {
			bool compatible = false;

			if (validConnectionClass == EABCapType::CCT_Belt) {
				// check belt type
				EFactoryConnectionDirection beltType = Cast<UFGFactoryConnectionComponent>(mSnappedConnection)->GetDirection();
				compatible =
					offsetMap[i].beltFilter == beltType ||
					offsetMap[i].beltFilter == EFactoryConnectionDirection::FCD_ANY ||
					beltType == EFactoryConnectionDirection::FCD_ANY;

			} else if (validConnectionClass == EABCapType::CCT_Pipe) {
				// check pipe type
				EPipeConnectionType pipeType = Cast<UFGPipeConnectionComponentBase>(mSnappedConnection)->GetPipeConnectionType();
				compatible =
					offsetMap[i].pipeFilter == pipeType ||
					offsetMap[i].pipeFilter == EPipeConnectionType::PCT_ANY ||
					pipeType == EPipeConnectionType::PCT_ANY;

			} else {
				UE_LOG(LogTemp, Error, TEXT("INVALID AB_CAP_TYPE USED"));
			}

			if (compatible) {
				//UE_LOG(LogTemp, Warning, TEXT("[X] [X] offset of %d using %d"), pIndex, i);
				AddActorLocalOffset(offsetMap[i].offsetRequired);
				break;
			}
		}
	}

	return true;
}

bool AABConveyorCapHologram::ValidatePotentialConnections() {
	int len = possibleConnections.Num();
	if (len < 1) { return false; }

	for (int i = 0; i < len; i++) {
		if (validConnectionClass == EABCapType::CCT_Belt) {
			UFGFactoryConnectionComponent* beltCnx = Cast<UFGFactoryConnectionComponent>(possibleConnections[i]);
			if (beltCnx != NULL && !beltCnx->IsConnected()) { continue; }
			// TODO: hide arrows and rings in hologram placement mode
			// TODO: hide arrows and rings in hologram placement mode
			// TODO: hide arrows and rings in hologram placement mode
			// TODO: hide arrows and rings in hologram placement mode

		} else if (validConnectionClass == EABCapType::CCT_Pipe) {
			UFGPipeConnectionComponentBase* pipeCnx = Cast<UFGPipeConnectionComponentBase>(possibleConnections[i]);
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

void AABConveyorCapHologram::ConfigureComponents(AFGBuildable* inBuildable) const {
	if (validConnectionClass == EABCapType::CCT_Belt) {
		UFGFactoryConnectionComponent* myCnx = inBuildable->GetComponentByClass<UFGFactoryConnectionComponent>();
		if (myCnx != NULL) {
			UFGFactoryConnectionComponent* theirCnx = Cast<UFGFactoryConnectionComponent>(mSnappedConnection);
			myCnx->SetConnection(theirCnx);
		}

	} else if (validConnectionClass == EABCapType::CCT_Pipe) {
		UFGPipeConnectionFactory* myCnx = inBuildable->GetComponentByClass<UFGPipeConnectionFactory>();
		if (myCnx != NULL) {
			UFGPipeConnectionComponentBase* theirCnx = Cast<UFGPipeConnectionComponentBase>(mSnappedConnection);
			myCnx->SetConnection(theirCnx);
		}

	} else {
		UE_LOG(LogTemp, Error, TEXT("INVALID AB_CAP_TYPE USED"));
	}

	Super::ConfigureComponents(inBuildable);
}