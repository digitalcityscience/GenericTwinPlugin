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

#pragma once

#include "CoreMinimal.h"
#include "Control/GenericTwinBasePlayerController.h"
#include "GeographicCoordinates.h"
#include "GenericTwinGlobePlayerController.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogGenericTwinGlobePlayerController, Log, All);

class AGeoReferencingSystem;


/**
 * 
 */
UCLASS()
class GENERICTWINCORE_API AGenericTwinGlobePlayerController : public AGenericTwinBasePlayerController
{
	GENERATED_BODY()

	enum class EControlMode : uint8
	{
		Idle,
		Zooming,
		DampZooming,
		Rotating,
		DampRotating,
		Panning,
		DampPanning,

	};

protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;

	virtual void Teleport(const FVector &Location, float Heading, float Pitch) override;

	virtual void TeleportToGeoLocation(const FVector &GeoLocation, float Heading, float Pitch) override;

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Control")
	void StartPan();

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Control")
	void StopPan();

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Control")
	void StartRotate();

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Control")
	void StopRotate();

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Control")
	void ZoomIn(float Delta);

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Control")
	void ZoomOut(float Delta);

	UPROPERTY(EditAnywhere, Category = "Configuration")
	FVector2D MouseSensitivity = FVector2D(15.0f, 8.0f);

	UPROPERTY(EditAnywhere, Category = "Configuration")
	TEnumAsByte<ECollisionChannel> LineTraceChannel = ECollisionChannel::ECC_Visibility;

	UPROPERTY(EditAnywhere, Category = "Configuration")
	TEnumAsByte<ECollisionChannel> CollisonTraceChannel = ECollisionChannel::ECC_Visibility;

	UPROPERTY(EditAnywhere, Category = "Configuration")
	float CollisionRadius = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Configuration")
	float OffsetThreshold = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Configuration|Zoom")
	bool StopZoomOnRotate = false;

	UPROPERTY(EditAnywhere, Category = "Configuration|Zoom")
	float RelativeZoomSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Configuration|Zoom")
	float ZoomDistanceFactor = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Configuration|Zoom")
	float ZoomIncreaseStep = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Configuration|Zoom")
	float MaxZoomMomentum = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Configuration|Zoom")
	float ZoomStopThreshold = 0.015f;

	UPROPERTY(EditAnywhere, Category = "Configuration|Panning")
	int32 PanHistorySamples = 5;

	UPROPERTY(EditAnywhere, Category = "Configuration|Panning")
	float PanDampingFactor = 0.9f;

private:

	void UpdateInput();

	void CorrectHeight(float DeltaSeconds);
	void CalculateGroundOffset(const FVector& Location);

	FRotator CorrectRotation(const FVector &UELocation);

	bool StartZoom();

	void ResetDamping(bool ResetZoom);

	bool CheckCollision(const FVector& From, const FVector& To);

	bool GetViewportLocationOnSurface(const FVector2D &ViewportPosition, FVector &LocationOnSurface);
	FVector GetViewportLocationOnSurface(const FVector2D &ViewportPosition);

	double CalcDistanceAndLocationOnSurface(FVector &Location, const FVector2D &ViewportOffset);
	int8 IsBelowGround(const FVector &Location, FVector *Offset = 0);

	double GetClosestVirtualGlobeIntersection(const FVector &RayStart, const FVector &RayDir, double SphereRadius);

	UPROPERTY()
	AGeoReferencingSystem			*m_GeoReferencing = 0;

	float							m_DesiredPitch = 0.0f;
	float							m_DesiredYaw = 0.0f;


	FVector2D						m_curMousePosition;
	FVector2D						m_prevMousePosition;
	FVector2D						m_curMouseDelta;

	EControlMode					m_ControlMode = EControlMode::Idle;

	FVector							m_ZoomAnchor;
	float							m_ZoomMomentum = 0.0f;

	FVector							m_PanAnchorECEF;
	FGeographicCoordinates			m_PanAnchorGeo;
	FVector							m_PanOffset;
	FGeographicCoordinates			m_PanInitialPawnPos;
	double							m_PanRadius = 0.0;
	FVector							m_PanMomentum = FVector::Zero();
	FVector							m_PrevPanDelta = FVector::Zero();
	TArray<FVector>					m_PanDeltaBuffer;
	int32							m_PanDeltaBufferIndex = 0;

	FSceneViewProjectionData		m_ProjectionData;
	FMatrix							m_InvViewProjMatrix;

	FVector							m_RotateAnchor;
	float							m_DeltaPitch = 0.0f;
	float							m_DeltaYaw = 0.0f;
	FVector							m_RotateOffset;

	FVector							m_RequiredOffset;
	FVector							m_curOffset;


	float							m_MouseX = 0.0f;
};
