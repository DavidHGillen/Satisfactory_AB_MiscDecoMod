#include "ABFenceHologram.h"
#include "Math/UnrealMathUtility.h"

#include "FGBuildableBeam.h"
#include "FGBuildablePillar.h"
#include "Buildables/FGBuildableWall.h"
#include "Buildables/FGBuildableRamp.h"

void AABFenceHologram::GetSupportedBuildModes_Implementation(TArray<TSubclassOf<UFGBuildGunModeDescriptor>>& out_buildmodes) const {
	Super::GetSupportedBuildModes_Implementation(out_buildmodes);

	if (mBuildModeGrounded) { out_buildmodes.AddUnique(mBuildModeGrounded); }
}

bool AABFenceHologram::DoMultiStepPlacement(bool isInputFromARelease) {
	if (IsCurrentBuildMode(mBuildModeGrounded)) { return true; }
	return Super::DoMultiStepPlacement(isInputFromARelease);
}

bool AABFenceHologram::TrySnapToActor(const FHitResult& hitResult) {
	AFGBuildable* hitBuildable = Cast<AFGBuildable>(hitResult.GetActor());

	// standard behaviour when snapping barriers
	if (hitBuildable != NULL && hitBuildable->mHologramClass->IsChildOf(this->GetClass())) {
		return Super::TrySnapToActor(hitResult);
	}

	// but don't snap elsewhere I've got it covered in location and rotation
	return false;
}

void AABFenceHologram::SetHologramLocationAndRotation(const FHitResult& hitResult) {
	// special cases
	if (IsCurrentBuildMode(mBuildModeGrounded)) {
		if (SetGroundedLocationAndRotation(hitResult)) {
			return;
		}
	}

	if (IsCurrentBuildMode(mBuildModeZoop)) {
		Super::SetHologramLocationAndRotation(hitResult);
		return;
	}

	AActor* hitActor = hitResult.GetActor();
	FRotator outRotation = FRotator::ZeroRotator;
	FVector outLocation = FVector::Zero();

	FTransform actorToWorld = hitActor->GetTransform();
	FTransform worldToActor = actorToWorld.Inverse();
	FVector localHitPosition = worldToActor.TransformPosition(hitResult.ImpactPoint);
	FVector localHitNormal = worldToActor.TransformVector(hitResult.ImpactNormal);

	// beams
	AFGBuildableBeam* hitBeam = Cast<AFGBuildableBeam>(hitActor);
	if (hitBeam != NULL) {
		// incase somone bevels the edges of the beam or something, snap to cardinal directions and local units
		float facing = FMath::Abs(localHitNormal.Y) > FMath::Abs(localHitNormal.Z) ? (localHitNormal.Y > 0.0f ? 90.0f : 270.0f) : (localHitNormal.Z > 0.0f ? 0.0f : 180.0f);
		outLocation = FVector(FMath::GridSnap(localHitPosition.X, mGridSnapSize), 0.0f, 0.0f);
		outRotation = FRotator(0.0f, 0.0f, facing);

		// place us where we should be in the world, and now we know we're where we should be facing the way we should be, move outside the beam
		SetActorLocationAndRotation(
			actorToWorld.TransformPosition(outLocation),
			actorToWorld.TransformRotation(outRotation.Quaternion())
		);
		AddActorLocalOffset(FVector(0.0f, 0.0f, hitBeam->GetSize() * 0.5f));
		AddActorLocalRotation(FRotator(0.0f, GetScrollRotateValue(), 0.0f));
		return;
	}

	// pillars
	AFGBuildablePillar* hitPillar = Cast<AFGBuildablePillar>(hitActor);
	if (hitPillar != NULL) {
		// pillars are guarenteed to be some rect with even dimensions
		outLocation = localHitPosition.GridSnap(mGridSnapSize);
		outRotation = localHitNormal.GridSnap(1.0f).Rotation();
		SetActorLocationAndRotation(
			actorToWorld.TransformPosition(outLocation),
			actorToWorld.TransformRotation(outRotation.Quaternion())
		);
		AddActorLocalRotation(FRotator(-90.0f, 0.0f, 0.0f));
		AddActorLocalRotation(FRotator(0.0f, GetScrollRotateValue(), 0.0f));
		return;
	}

	// foundations enable side snapping and handle underside snapping, also follow slopes
	AFGBuildableFoundation* hitFoundation = Cast<AFGBuildableFoundation>(hitActor);
	if (hitFoundation != NULL) {
		FVector absLocalNormal = localHitNormal.GetAbs();
		bool nonCardinal = absLocalNormal.X < 0.998f && absLocalNormal.Y < 0.998f && absLocalNormal.Z < 0.998f;
		bool onTheUnderside = localHitNormal.Z < 0.0f;

		AFGBuildableRamp* hitRamp = Cast<AFGBuildableRamp>(hitActor);
		if (hitRamp != NULL && nonCardinal) {
			// ramp's ramped surface enjoy special behaviours not of the utility
			float slopeHigh = hitRamp->mElevation * 0.5f;
			float slopeLow = hitRamp->mElevation * -0.5f;

			if (hitRamp->mIsDoubleRamp && onTheUnderside) {
				// double ramps are basically single ramps with an offset underneath ramp
				slopeLow -= hitRamp->mHeight;
				slopeHigh -= hitRamp->mHeight;
			} else if (hitRamp->mIsRoof) {
				// roofs are thick and about the same as walls so lets reuse that offset
				slopeLow += onTheUnderside ? -wallOffset : wallOffset;
				slopeHigh += onTheUnderside ? -wallOffset : wallOffset;
			}

			float slopeAngle = FMath::RadiansToDegrees(atan2(slopeHigh - slopeLow, hitRamp->mDepth)); //hitRamp->CalculateRampAngle()

			outLocation = localHitPosition.GridSnap(mGridSnapSize);
			outRotation = FRotator(-slopeAngle, 0.0f, onTheUnderside ? 180.0f : 0.0f);
			outLocation.Z = FMath::Lerp(slopeHigh, slopeLow, outLocation.X / hitRamp->mDepth + 0.5f);

			SetActorLocationAndRotation(
				actorToWorld.TransformPosition(outLocation),
				actorToWorld.TransformRotation(outRotation.Quaternion())
			);
		} else {
			// the default utility is fine here
			outLocation = hitResult.ImpactPoint;
			SnapToFoundationSide(hitFoundation, localHitNormal, EAxis::Z, outLocation, outRotation);
			SetActorLocationAndRotation(
				outLocation,
				outRotation
			);
		}

		AddActorLocalRotation(FRotator(0.0f, GetScrollRotateValue(), 0.0f));
		return;
	}

	// normal behaviour for remaining things
	Super::SetHologramLocationAndRotation(hitResult);
}

bool AABFenceHologram::SetGroundedLocationAndRotation(const FHitResult& hitResult) {
	UWorld* zeWorld = GetWorld();
	FRotator outRotation = FRotator(0.0f, GetScrollRotateValue(), 0.0f);
	AFGBuildableWall* myWall = GetDefaultBuildable<AFGBuildableWall>();
	float fLength = myWall == NULL ? 400.0f : myWall->mWidth;
	float fStep = fLength / iAccuracy;
	float fStart = fLength * -0.5 + fStep * 0.5;

	// Initialize it to the correct spot so we can use it as a reference point
	FTransform actorToWorld = FTransform::Identity;
	actorToWorld.SetRotation(outRotation.Quaternion());
	actorToWorld.SetTranslation(hitResult.ImpactPoint);
	FTransform worldToActor = actorToWorld.Inverse();
	TArray<FVector> foundHits;

	// Find all the valid hits below, this is the right number of times to run it because of the offset
	for (int i=0, l=iAccuracy; i<l; i++) {
		FHitResult tempHit;
		bool hit = zeWorld->LineTraceSingleByChannel(
			tempHit,
			actorToWorld.TransformPosition(FVector(0, fStart + fStep*i, vertRange_Up)),
			actorToWorld.TransformPosition(FVector(0, fStart + fStep*i, -vertRange_Dn)),
			eChannel);

		if (hit) {
			//UE_LOG(LogTemp, Warning, TEXT("=== dot: %f %f %f"), tempHit.ImpactPoint.X, tempHit.ImpactPoint.Y, tempHit.ImpactPoint.Z);
			foundHits.Add(worldToActor.TransformPosition(tempHit.ImpactPoint));
		}
	}

	// Without two points we can't do anything novel
	if (foundHits.Num() < 2) { return false; }

	float fGroundLength = foundHits.Last().Y - foundHits[0].Y;
	float fRadianSlope = atan2(foundHits.Last().Z - foundHits[0].Z, fGroundLength);

	//UE_LOG(LogTemp, Warning, TEXT("=== %f %f"), fGroundLength, FMath::RadiansToDegrees(fRadianSlope));

	SetActorLocationAndRotation(
		hitResult.ImpactPoint - FVector(0.0f, 0.0f, (tan(fRadianSlope) * foundHits[0].Y - foundHits[0].Z)),
		outRotation
	);
	AddActorLocalRotation(FRotator(0.0f, 0.0f, -FMath::RadiansToDegrees(fRadianSlope)).Quaternion());

	return true;
}