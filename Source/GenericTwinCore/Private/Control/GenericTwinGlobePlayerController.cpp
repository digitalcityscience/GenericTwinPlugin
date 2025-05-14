/**

The MIT License (MIT)

Copyright (c) 2023 - present, Hamburg Port Authority


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

**/

#include "Control/GenericTwinGlobePlayerController.h"

#include "GenericTwin.h"
#include "TwinWorldContext.h"

#include "Math/RotationMatrix.h"
#include "Kismet/KismetMathLibrary.h"
#include "GeoReferencingSystem.h"

DEFINE_LOG_CATEGORY(LogGenericTwinGlobePlayerController);

void AGenericTwinGlobePlayerController::BeginPlay()
{
	Super::BeginPlay();
	m_GeoReferencing = AGenericTwin::GetGenericTwin(GetWorld())->GetWorldContext().GeoReferencingSystem;
}

void AGenericTwinGlobePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if	(	m_GeoReferencing != nullptr
		&&	GenericTwinPawn != nullptr
		)
	{
		UpdateInput();

		if(FMath::Abs(m_ZoomMomentum) > 0.05f)
		{
			FVector ctr;
			double distance = CalcDistanceAndLocationOnSurface(ctr);

			FVector direction = m_ZoomAnchor - GenericTwinPawn->GetActorLocation();
			direction.Normalize();

			FVector newPos = GenericTwinPawn->GetActorLocation() + m_ZoomMomentum * 0.3 * distance * DeltaTime * direction;
			GenericTwinPawn->SetActorLocationAndRotation(newPos, CorrectRotation(newPos));

			const float referenceDeltaTime = 1.0f / 30.0f;
			m_ZoomMomentum *= FMath::Pow(0.975f, DeltaTime / referenceDeltaTime);
		}
		else
		{
			m_ZoomMomentum = 0.0f;
		}

		if(m_isRotating)
		{
			FVector east;
			FVector north;
			FVector up;
			m_GeoReferencing->GetENUVectorsAtEngineLocation(m_RotateAnchor, east, north, up);

			FQuat baseRotWS = UKismetMathLibrary::MakeRotationFromAxes(east, -north, up).Quaternion();	//	change to ESU to match Unreal's coordinate system

			FVector offsetWS = GenericTwinPawn->GetActorLocation() - m_RotateAnchor;

			// use original up vector as yaw axis
			const FQuat yawDelta = FQuat(up, FMath::DegreesToRadians(m_curMouseDelta.X * MouseSensitivity.X));

			offsetWS = yawDelta.RotateVector(offsetWS);

			// Apply yaw to the pawn's orientation
			FQuat curPawnRotationWS = GenericTwinPawn->GetActorQuat();
			curPawnRotationWS = yawDelta * curPawnRotationWS;
			curPawnRotationWS.Normalize(); // Keep the quaternion normalized

			// Pitch axis is the "right" vector of the pawn *after* the yaw and *considering* base world rotation.
			const FVector pitchAxisWS = curPawnRotationWS.GetRightVector();
			const FQuat pitchDelta = FQuat(pitchAxisWS, FMath::DegreesToRadians(m_curMouseDelta.Y * MouseSensitivity.Y));

			// Rotate the (already yawed) offset vector
			offsetWS = pitchDelta.RotateVector(offsetWS);
			// Apply pitch to the pawn's (already yawed) orientation
			curPawnRotationWS = pitchDelta * curPawnRotationWS;
			curPawnRotationWS.Normalize();

			FVector newPos = m_RotateAnchor + offsetWS;

			GenericTwinPawn->SetActorLocationAndRotation(newPos, curPawnRotationWS.Rotator());

			// extract local pitch and yaw from current world rotation
			FRotator deltaRot = CalculateRelativeRotation(curPawnRotationWS, newPos);
			m_DesiredPitch = deltaRot.Pitch;
			m_DesiredYaw = deltaRot.Yaw;
		}

		if(m_isPanning)
		{
#if 1
			FVector target = GetViewportLocationOnSurface(m_curMousePosition);
			FVector prevTarget = GetViewportLocationOnSurface(m_prevMousePosition);

			FGeographicCoordinates prevGeo;
			FGeographicCoordinates targetGeo;
			m_GeoReferencing->EngineToGeographic(prevTarget, prevGeo);
			m_GeoReferencing->EngineToGeographic(target, targetGeo);

			FGeographicCoordinates pawnGeo;
			m_GeoReferencing->EngineToGeographic(GenericTwinPawn->GetActorLocation(), pawnGeo);
			pawnGeo.Longitude += prevGeo.Longitude - targetGeo.Longitude;
			pawnGeo.Latitude += prevGeo.Latitude - targetGeo.Latitude;
			pawnGeo.Altitude += prevGeo.Altitude - targetGeo.Altitude;

			FVector newPos;
			m_GeoReferencing->GeographicToEngine(pawnGeo, newPos);
			GenericTwinPawn->SetActorLocationAndRotation(newPos, CorrectRotation(newPos));

#else

			FVector target = GetViewportLocationOnSurface(m_curMousePosition);
			FVector prevTarget = GetViewportLocationOnSurface(m_prevMousePosition);

			FVector delta = prevTarget - target;

			FVector newPos = GenericTwinPawn->GetActorLocation() + delta;
			// SetControlRotation(CorrectRotation(newPos));
			// GenericTwinPawn->SetActorLocation(newPos);
			GenericTwinPawn->SetActorLocationAndRotation(newPos, CorrectRotation(newPos));
			// UE_LOG(LogGenericTwinMaterialManager, Log, TEXT("MousePos %s NewPos %s Delta %s"), *(m_curMousePosition.ToString()), *(newPos.ToString()), *(delta.ToString()));

			// FVector delta = m_PanAnchor - target;

			// m_PanDelta += delta;

			// FVector newPos = GenericTwinPawn->GetActorLocation() + delta;
			//SetControlRotation(CorrectRotation(newPos));
			// GenericTwinPawn->SetActorLocation(newPos);

			// UE_LOG(LogGenericTwinMaterialManager, Log, TEXT("NewPos %s Delta %s"), *(newPos.ToString()), *(delta.ToString()));

			m_PanAnchor = target;

#endif
			FVector dummy;
			double distance = CalcDistanceAndLocationOnSurface(dummy);
			UE_LOG(LogGenericTwinMaterialManager, Log, TEXT("Distance %f"), distance);

		}
	}

}

void AGenericTwinGlobePlayerController::Teleport(const FVector &Location, float Heading, float Pitch)
{
	if (GenericTwinPawn)
	{
		m_DesiredPitch = Pitch;
		m_DesiredYaw = Heading;
		GenericTwinPawn->SetActorLocationAndRotation(Location, CorrectRotation(Location));
	}
}

void AGenericTwinGlobePlayerController::TeleportToGeoLocation(const FVector &GeoLocation, float Heading, float Pitch)
{
	if (GenericTwinPawn)
	{
		m_DesiredPitch = Pitch;
		m_DesiredYaw = Heading;

		FVector ueLocation;
		AGenericTwin::GetGenericTwin(GetWorld())->GetWorldContext().GeoReferencingSystem->GeographicToEngine(FGeographicCoordinates(GeoLocation.X, GeoLocation.Y, GeoLocation.Z), ueLocation);
		GenericTwinPawn->SetActorLocationAndRotation(ueLocation, CorrectRotation(ueLocation));
	}
}

void AGenericTwinGlobePlayerController::StartPan()
{
	m_PanAnchor = GetViewportLocationOnSurface(m_curMousePosition);
	m_PanDelta = FVector::Zero();
	m_isPanning = true;
	m_ZoomMomentum = 0.0f;
	UE_LOG(LogGenericTwinMaterialManager, Verbose, TEXT("StartPan %s"), *(GenericTwinPawn->GetActorLocation().ToString()) );
}

void AGenericTwinGlobePlayerController::StopPan()
{
	m_isPanning = false;
	m_ZoomMomentum = 0.0f;
	UE_LOG(LogGenericTwinMaterialManager, Verbose, TEXT("StopPan %s"), *(GenericTwinPawn->GetActorLocation().ToString()) );
}

void AGenericTwinGlobePlayerController::StartRotate()
{
	m_isRotating = true;
	m_RotateAnchor = GetViewportLocationOnSurface(m_curMousePosition);
}

void AGenericTwinGlobePlayerController::StopRotate()
{
	m_isRotating = false;
}

void AGenericTwinGlobePlayerController::ZoomIn(float Delta)
{
	if(StartZoom())
	{
		m_ZoomMomentum = 1.0f;
	}
}

void AGenericTwinGlobePlayerController::ZoomOut(float Delta)
{
	if(StartZoom())
	{
		m_ZoomMomentum = -1.0f;
	}
}


bool AGenericTwinGlobePlayerController::StartZoom()
{
	m_ZoomAnchor = GetViewportLocationOnSurface(m_curMousePosition);
	return !m_isPanning;
}

FRotator AGenericTwinGlobePlayerController::CorrectRotation(const FVector &UELocation)
{
	FVector east;
	FVector north;
	FVector up;
	m_GeoReferencing->GetENUVectorsAtEngineLocation(UELocation, east, north, up);
	UE_LOG(LogGenericTwinMaterialManager, Verbose, TEXT("CorrectRotation E %s N %s U %s"), *(east.ToString()), *(north.ToString()), *(up.ToString()));
	const FRotator baseRotWS = UKismetMathLibrary::MakeRotationFromAxes(east, -north, up);
	const FRotator localRot = FRotator(m_DesiredPitch, m_DesiredYaw, 0.0);
	return UKismetMathLibrary::ComposeRotators(localRot, baseRotWS);
}

bool AGenericTwinGlobePlayerController::CheckCollision(const FVector& From, const FVector& To)
{
	FHitResult hitResult;
	FCollisionShape collisionShape;
	collisionShape.ShapeType = ECollisionShape::Sphere;
	collisionShape.Sphere.Radius = CollisionRadius;
	FCollisionQueryParams queryParams;
	queryParams.AddIgnoredActor(this);
	FCollisionResponseParams responseParams;
	FVector dir(To - From);
	dir.Normalize();
	FVector traceStart = GenericTwinPawn->GetActorLocation() + dir * (CollisionRadius * 1.005);
	return GetWorld()->SweepSingleByChannel(hitResult, traceStart, To, FQuat(), CollisonTraceChannel, collisionShape, queryParams, responseParams);
}

FVector AGenericTwinGlobePlayerController::GetViewportLocationOnSurface(const FVector2D &ViewportPosition)
{
	FVector start;
	FVector direction;
	if(DeprojectScreenPositionToWorld(ViewportPosition.X, ViewportPosition.Y, start, direction))
	{
		FVector center;
		m_GeoReferencing->ECEFToEngine(FVector::Zero(), center);
		const double distance = FVector::Distance(GenericTwinPawn->GetActorLocation(), center);
		const FVector end = start + distance * direction;
		FCollisionQueryParams params;
		params.AddIgnoredActor(this);
		FHitResult hitResult;
		if (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, LineTraceChannel, params))
		{
			return hitResult.ImpactPoint;
		}
	}

	return FVector::Zero();
}


void AGenericTwinGlobePlayerController::UpdateInput()
{
	m_prevMousePosition = m_curMousePosition;
	GetLocalPlayer()->ViewportClient->GetMousePosition(m_curMousePosition);
	m_curMouseDelta = m_curMousePosition - m_prevMousePosition;
}

double AGenericTwinGlobePlayerController::CalcDistanceAndLocationOnSurface(FVector &Location)
{
	FVector2D viewportCenter;
	GEngine->GameViewport->GetViewportSize(viewportCenter);
	Location = GetViewportLocationOnSurface(0.5 * viewportCenter);
	return Location == FVector::Zero() ? 0.0 : FVector::Distance(GenericTwinPawn->GetActorLocation(), Location);

}

FRotator AGenericTwinGlobePlayerController::CalculateRelativeRotation(const FQuat& RotationB, const FVector& RefLocation)
{
	FTransform refTransform = m_GeoReferencing->GetTangentTransformAtEngineLocation(RefLocation);
	FQuat rotationA = refTransform.ToMatrixNoScale().ToQuat();

	FQuat relRot = rotationA.Inverse() * RotationB;
	return relRot.Rotator();
}
