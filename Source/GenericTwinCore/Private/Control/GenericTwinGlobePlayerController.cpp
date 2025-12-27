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
	m_PanDeltaBuffer.Reserve(PanHistorySamples);
	for (int32 i = 0; i < PanHistorySamples; ++i)
		m_PanDeltaBuffer.Add(FVector::ZeroVector);
}

void AGenericTwinGlobePlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if	(	m_GeoReferencing != nullptr
		&&	GenericTwinPawn != nullptr
		)
	{
		UpdateInput();

		switch (m_ControlMode)
		{
			case EControlMode::Zooming:
			case EControlMode::DampZooming:
				if(FMath::Abs(m_ZoomMomentum) > ZoomStopThreshold)
				{
					const FVector curPos = GenericTwinPawn->GetActorLocation();

					FVector ctr;
					const double refDistance = CalcDistanceAndLocationOnSurface(ctr, FVector2D(0.5, 0.8));
					const double anchorDistance = FVector::Distance(m_ZoomAnchor, curPos);
					const double distance = FMath::Min(ZoomDistanceFactor * refDistance, anchorDistance);

					FVector direction = m_ZoomAnchor - curPos;
					direction.Normalize();

					FVector newPos = curPos + m_ZoomMomentum * RelativeZoomSpeed * anchorDistance * DeltaTime * direction;
					GenericTwinPawn->SetActorLocationAndRotation(newPos, CorrectRotation(newPos));

					const float referenceDeltaTime = 1.0f / 30.0f;
					m_ZoomMomentum *= FMath::Pow(0.975f, DeltaTime / referenceDeltaTime);
					m_ControlMode = EControlMode::DampZooming;
				}
				else
				{
					m_ZoomMomentum = 0.0f;
					m_ControlMode = EControlMode::Idle;
				}
				break;

			case EControlMode::Rotating:
				{
					// extract local axis
					FVector east;
					FVector north;
					FVector up;
					m_GeoReferencing->GetENUVectorsAtEngineLocation(m_RotateAnchor, east, north, up);

					// process input
					const float inputYawDelta = m_curMouseDelta.X * MouseSensitivity.X * DeltaTime;
					float inputPitchDelta = m_curMouseDelta.Y * MouseSensitivity.Y * DeltaTime;

					const FVector2D PitchRange(-90.0, 0.0);

					float updatedPitch = m_DesiredPitch - inputPitchDelta;

					if(updatedPitch < PitchRange.X)
					{
						inputPitchDelta -= PitchRange.X - updatedPitch;
					}
					else if(updatedPitch > PitchRange.Y)
					{
						inputPitchDelta += updatedPitch - PitchRange.Y;
					}

					// backup current rotation values and prepare new ones
					const float prevDesiredYaw = m_DesiredYaw;
					const float preDesiredPitch = m_DesiredPitch;

					const float newDeltaYaw = m_DeltaYaw + inputYawDelta;
					const float newDeltaPitch = m_DeltaPitch + inputPitchDelta;

					// setup "absolute" yaw rotation around current up axis
					const FQuat yawDelta = FQuat(up, FMath::DegreesToRadians(newDeltaYaw));
					FVector offset = yawDelta.RotateVector(m_RotateOffset);
					m_DesiredYaw += inputYawDelta;

					// setup "absolute" pitch rotation but around *new* right axis
					const FVector curRight = CorrectRotation(m_RotateAnchor + offset).Quaternion().GetRightVector();
					const FQuat pitchDelta = FQuat(curRight, FMath::DegreesToRadians(newDeltaPitch));
					offset = pitchDelta.RotateVector(offset);
					m_DesiredPitch -= inputPitchDelta;

					// calculate new position and check for potential collision
					FVector newPos = m_RotateAnchor + offset;
					if(CheckCollision(GenericTwinPawn->GetActorLocation(), newPos))
					{
						// collision occured, ignore new position and restore yaw/pitch
						m_DesiredYaw = prevDesiredYaw;
						m_DesiredPitch = preDesiredPitch;	
					}
					else
					{
						//	move pawn to new position and update local yaw/pitch
						GenericTwinPawn->SetActorLocationAndRotation(newPos, CorrectRotation(newPos));
						m_DeltaYaw = newDeltaYaw;
						m_DeltaPitch = newDeltaPitch;
					}

				}
				break;

			case EControlMode::DampRotating:
				break;

			case EControlMode::Panning:
				//if (m_curMouseDelta.SquaredLength() > 0.0)
				{
					// get view direction based on initial camera position/orientation
					FVector viewStart;
					FVector viewDirection;
					FSceneView::DeprojectScreenToWorld(m_curMousePosition, m_ProjectionData.GetConstrainedViewRect(), m_InvViewProjMatrix, viewStart, viewDirection);

					// convert view direction from engine space to ECEF by using temporary position
					FVector curPosECEF;
					m_GeoReferencing->EngineToECEF(viewStart, curPosECEF);
					FVector tmpPos = viewStart + 10000.0 * viewDirection;
					FVector tmpPosECEF;
					m_GeoReferencing->EngineToECEF(tmpPos, tmpPosECEF);
					viewDirection = tmpPosECEF - curPosECEF;
					viewDirection.Normalize();

					UE_LOG(LogGenericTwinGlobePlayerController, Log, TEXT("MouseDelta %s"), *(m_curMouseDelta.ToString()));


					// intersect with "virtual" sphere
					double t = GetClosestVirtualGlobeIntersection(curPosECEF, viewDirection, m_PanRadius);
					if(t != 0.0)
					{
						//	calculate target position on vritual sphere in ECEF where mouse pointer is currently pointing to
						FVector targetECEF = curPosECEF + t * viewDirection;

						FGeographicCoordinates targetGeo;
						m_GeoReferencing->ECEFToGeographic(targetECEF, targetGeo);

						// check whether pawm is currently inside or outside of virtual sphere
						m_GeoReferencing->EngineToECEF(GenericTwinPawn->GetActorLocation(), curPosECEF);
						const double sign = curPosECEF.Length() < m_PanRadius ? -1.0 : 1.0;

						// calculate geographical "absolute" delta and calculate new (geographical) position
						FVector delta(m_PanAnchorGeo.Longitude - targetGeo.Longitude, m_PanAnchorGeo.Latitude - targetGeo.Latitude, 0.0);
						delta *= sign;
						FGeographicCoordinates pawnGeo;
						m_GeoReferencing->EngineToGeographic(GenericTwinPawn->GetActorLocation(), pawnGeo);
						pawnGeo.Longitude = m_PanInitialPawnPos.Longitude + delta.X;
						pawnGeo.Latitude = m_PanInitialPawnPos.Latitude + delta.Y;

						m_PanDeltaBuffer[m_PanDeltaBufferIndex % PanHistorySamples] = delta - m_PrevPanDelta;
						++m_PanDeltaBufferIndex;

						m_PrevPanDelta = delta;

						// convert new position to engine space check for collision
						FVector newPos;
						m_GeoReferencing->GeographicToEngine(pawnGeo, newPos);
						if(CheckCollision(GenericTwinPawn->GetActorLocation(), newPos) == false)
						{
							// no collision, update pawn's location
							GenericTwinPawn->SetActorLocationAndRotation(newPos, CorrectRotation(newPos));
						}
					}
				}
				break;

			case EControlMode::DampPanning:
				{
					FGeographicCoordinates pawnGeo;
					m_GeoReferencing->EngineToGeographic(GenericTwinPawn->GetActorLocation(), pawnGeo);
					pawnGeo.Longitude += m_PanMomentum.X;
					pawnGeo.Latitude += m_PanMomentum.Y;

					// convert new position to engine space check for collision
					FVector newPos;
					m_GeoReferencing->GeographicToEngine(pawnGeo, newPos);
					if(CheckCollision(GenericTwinPawn->GetActorLocation(), newPos) == false)
					{
						// no collision, update pawn's location
						GenericTwinPawn->SetActorLocationAndRotation(newPos, CorrectRotation(newPos));
					}

					const float referenceDeltaTime = 1.0f / 30.0f;
					m_PanMomentum *= FMath::Pow(PanDampingFactor, DeltaTime / referenceDeltaTime);
					if(m_PanMomentum.Length() < 0.0001)
					{
						m_ControlMode = EControlMode::Idle;
					}
				}
				break;
		}


		// apply correction to pawn's height in case it ended up under ground
		CorrectHeight(DeltaTime);
	}
}

double AGenericTwinGlobePlayerController::GetClosestVirtualGlobeIntersection(const FVector &RayStart, const FVector &RayDir, double SphereRadius)
{
	const double a = FVector::DotProduct(RayDir, RayDir);
	const double b = 2.0 * FVector::DotProduct(RayStart, RayDir);
	const double c = FVector::DotProduct(RayStart, RayStart) - SphereRadius * SphereRadius;

	const double D = b * b - 4.0 * a * c;
	if(D >= 0)
	{
		const double t0 = (-b - FMath::Sqrt(D)) / 2.0 * a;
		const double t1 = (-b + FMath::Sqrt(D)) / 2.0 * a;
		return t0 > 0.0 ? t0 : t1;
	}

	return 0.0;
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

void AGenericTwinGlobePlayerController::CorrectHeight(float DeltaSeconds)
{
	const FVector location = GenericTwinPawn->GetActorLocation();
	CalculateGroundOffset(location);
	const double length = m_RequiredOffset.Length();
	if(length > 0.0)
	{
		m_curOffset =	length <= OffsetThreshold
					?	m_RequiredOffset
					:	FMath::VInterpTo(m_curOffset, m_RequiredOffset, DeltaSeconds, 6.0f)
					;

		GenericTwinPawn->SetActorLocation(location + m_curOffset);
		UE_LOG(LogGenericTwinGlobePlayerController, Verbose, TEXT("Location %s CurOffset %s ReqOffset %s"), *(location.ToString()), *(m_curOffset.ToString()), *(m_RequiredOffset.ToString()));

	}
}

void AGenericTwinGlobePlayerController::StartPan()
{
	if	(	m_ControlMode == EControlMode::Idle
		||	m_ControlMode == EControlMode::DampZooming
		||	m_ControlMode == EControlMode::DampRotating
		||	m_ControlMode == EControlMode::DampPanning
		)
	{
		ResetDamping(true);

		FVector anchor;
		if(GetViewportLocationOnSurface(m_curMousePosition, anchor))
		{
			m_GeoReferencing->EngineToGeographic(anchor, m_PanAnchorGeo);
			m_GeoReferencing->EngineToECEF(anchor, m_PanAnchorECEF);
			m_PanRadius = m_PanAnchorECEF.Length();
			m_PrevPanDelta = FVector::Zero();

			m_GeoReferencing->EngineToGeographic(GenericTwinPawn->GetActorLocation(), m_PanInitialPawnPos);

			const ULocalPlayer *LocalPlayer = GetLocalPlayer();
			if (LocalPlayer && LocalPlayer->ViewportClient)
			{
				if (LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, m_ProjectionData))
				{
					m_InvViewProjMatrix = m_ProjectionData.ComputeViewProjectionMatrix().InverseFast();
					m_ControlMode = EControlMode::Panning;
					UE_LOG(LogGenericTwinGlobePlayerController, Log, TEXT("StartPan PanRadius %f"), m_PanRadius );
				}
			}
		}
	}
}

void AGenericTwinGlobePlayerController::StopPan()
{
	if(m_ControlMode == EControlMode::Panning)
	{
		m_ControlMode = EControlMode::DampPanning;
		m_PanInitialPawnPos.Longitude += m_PanMomentum.X;
		m_PanInitialPawnPos.Latitude += m_PanMomentum.Y;
		m_PanMomentum = FVector::Zero();
		const int32 numSamples = FMath::Min(PanHistorySamples, m_PanDeltaBufferIndex);
		for (int32 i = 0; i < numSamples; ++i)
			m_PanMomentum += m_PanDeltaBuffer[i];
		m_PanMomentum /= static_cast<float> (numSamples);

		UE_LOG(LogGenericTwinGlobePlayerController, Verbose, TEXT("StopPan %s"), *(GenericTwinPawn->GetActorLocation().ToString()));
	}
}

void AGenericTwinGlobePlayerController::StartRotate()
{
	if	(	m_ControlMode == EControlMode::Idle
		||	m_ControlMode == EControlMode::DampZooming
		||	m_ControlMode == EControlMode::DampRotating
		||	m_ControlMode == EControlMode::DampPanning
		)
	{
		ResetDamping(true);
		m_RotateAnchor = GetViewportLocationOnSurface(m_curMousePosition);

		const FVector curPos = GenericTwinPawn->GetActorLocation();
		m_DeltaPitch = 0.0f;
		m_DeltaYaw = 0.0f;
		m_RotateOffset = curPos - m_RotateAnchor;
		m_ControlMode = EControlMode::Rotating;
	}
}

void AGenericTwinGlobePlayerController::StopRotate()
{
	if (m_ControlMode == EControlMode::Rotating)
	{
		m_ControlMode = EControlMode::DampRotating;
	}
}

void AGenericTwinGlobePlayerController::ZoomIn(float Delta)
{
	if	(	m_ControlMode == EControlMode::Idle
		||	m_ControlMode == EControlMode::Zooming
		||	m_ControlMode == EControlMode::DampZooming
		||	m_ControlMode == EControlMode::DampRotating
		||	m_ControlMode == EControlMode::DampPanning
		)
	{
		ResetDamping(false);

		if(m_ZoomMomentum < 0.0f)
		{
			m_ZoomMomentum = 0.0f;
		}
		else if(StartZoom())
		{
			m_ZoomMomentum = FMath::Min(m_ZoomMomentum + ZoomIncreaseStep, MaxZoomMomentum);
		}
		m_ControlMode = EControlMode::Zooming;
	}
}

void AGenericTwinGlobePlayerController::ZoomOut(float Delta)
{
	if	(	m_ControlMode == EControlMode::Idle
		||	m_ControlMode == EControlMode::Zooming
		||	m_ControlMode == EControlMode::DampZooming
		||	m_ControlMode == EControlMode::DampRotating
		||	m_ControlMode == EControlMode::DampPanning
		)
	{
		ResetDamping(false);
		if(m_ZoomMomentum > 0.0f)
		{
			m_ZoomMomentum = 0.0f;
		}
		else if(StartZoom())
		{
			m_ZoomMomentum = FMath::Max(m_ZoomMomentum - ZoomIncreaseStep, -MaxZoomMomentum);
		}
		m_ControlMode = EControlMode::Zooming;
	}
}

bool AGenericTwinGlobePlayerController::StartZoom()
{
	return GetViewportLocationOnSurface(m_curMousePosition, m_ZoomAnchor);

	// m_ZoomAnchor = GetViewportLocationOnSurface(m_curMousePosition);
	// return !m_isPanning;
}

void AGenericTwinGlobePlayerController::ResetDamping(bool ResetZoom)
{
	if(ResetZoom)
		m_ZoomMomentum = 0.0f;
	m_PrevPanDelta = FVector::Zero();
	m_PanMomentum = FVector::Zero();
	m_PanDeltaBufferIndex = 0;
}

FRotator AGenericTwinGlobePlayerController::CorrectRotation(const FVector &UELocation)
{
	FVector east;
	FVector north;
	FVector up;
	m_GeoReferencing->GetENUVectorsAtEngineLocation(UELocation, east, north, up);
	UE_LOG(LogGenericTwinGlobePlayerController, Verbose, TEXT("CorrectRotation E %s N %s U %s"), *(east.ToString()), *(north.ToString()), *(up.ToString()));
	const FRotator baseRotWS = UKismetMathLibrary::MakeRotationFromAxes(east, -north, up);
	const FRotator localRot = FRotator(m_DesiredPitch, m_DesiredYaw, 0.0);
	return UKismetMathLibrary::ComposeRotators(localRot, baseRotWS);
}

void AGenericTwinGlobePlayerController::CalculateGroundOffset(const FVector& Location)
{
	FVector east;
	FVector north;
	FVector up;
	m_GeoReferencing->GetENUVectorsAtEngineLocation(Location, east, north, up);

	const double distance = 1000000.0;
	const FVector start = Location + distance * up;
	const FVector end = Location - distance * up;


	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	FHitResult hitResult;
	if (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, LineTraceChannel, params))
	{
		if(Location.Z < hitResult.ImpactPoint.Z)
		{
			FVector alteredPos(hitResult.ImpactPoint);
			alteredPos += hitResult.ImpactNormal * CollisionRadius * 1.5;
			m_RequiredOffset = alteredPos - Location;
		}
		else
		{
			m_curOffset = FVector::Zero();
			m_RequiredOffset = FVector::Zero();
		}
	}
	else
	{
		m_curOffset = FVector::Zero();
	}

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
	const bool hasCollided = GetWorld()->SweepSingleByChannel(hitResult, traceStart, To, FQuat(), CollisonTraceChannel, collisionShape, queryParams, responseParams);
	if(hasCollided == true)
	{
		return true;
	}

	return false;
}

bool AGenericTwinGlobePlayerController::GetViewportLocationOnSurface(const FVector2D &ViewportPosition, FVector &LocationOnSurface)
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
			LocationOnSurface = hitResult.ImpactPoint;
			return true;
		}
	}

	return false;
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

double AGenericTwinGlobePlayerController::CalcDistanceAndLocationOnSurface(FVector &Location, const FVector2D &ViewportOffset)
{
	FVector2D viewportCenter;
	GEngine->GameViewport->GetViewportSize(viewportCenter);
	Location = GetViewportLocationOnSurface(ViewportOffset * viewportCenter);
	return Location == FVector::Zero() ? 0.0 : FVector::Distance(GenericTwinPawn->GetActorLocation(), Location);

}

int8 AGenericTwinGlobePlayerController::IsBelowGround(const FVector &Location, FVector *Offset)
{
	FVector east;
	FVector north;
	FVector up;
	m_GeoReferencing->GetENUVectorsAtEngineLocation(Location, east, north, up);

	const double distance = 1000000.0;
	const FVector start = Location + distance * up;
	const FVector end = Location - distance * up;

	FCollisionQueryParams params;
	params.AddIgnoredActor(this);
	FHitResult hitResult;
	if (GetWorld()->LineTraceSingleByChannel(hitResult, start, end, LineTraceChannel, params))
	{
		if(Offset != nullptr)
		{
			*Offset = hitResult.ImpactPoint - Location;
		}
		return Location.Z < hitResult.ImpactPoint.Z ? -1 : 1;
	}

	return 0;
}
