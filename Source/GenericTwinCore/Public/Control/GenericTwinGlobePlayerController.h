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
	FVector2D MouseSensitivity = FVector2D(1.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Configuration")
	TEnumAsByte<ECollisionChannel> LineTraceChannel = ECollisionChannel::ECC_Visibility;

	UPROPERTY(EditAnywhere, Category = "Configuration")
	TEnumAsByte<ECollisionChannel> CollisonTraceChannel = ECollisionChannel::ECC_Visibility;

	UPROPERTY(EditAnywhere, Category = "Configuration")
	float CollisionRadius = 50.0f;

private:

	void UpdateInput();

	FRotator CorrectRotation(const FVector &UELocation);

	bool StartZoom();
	bool CheckCollision(const FVector& From, const FVector& To);

	FVector GetViewportLocationOnSurface(const FVector2D &ViewportPosition);

	double CalcDistanceAndLocationOnSurface(FVector &Location);

	FRotator CalculateRelativeRotation(const FQuat& RotationB, const FVector& RefLocation);

	UPROPERTY()
	AGeoReferencingSystem			*m_GeoReferencing = 0;

	float							m_DesiredPitch = 0.0f;
	float							m_DesiredYaw = 0.0f;

	FVector2D						m_curMousePosition;
	FVector2D						m_prevMousePosition;
	FVector2D						m_curMouseDelta;

	FVector							m_ZoomAnchor;
	float							m_ZoomMomentum = 0.0f;

	FVector							m_PanAnchor;
	FVector							m_PanDelta;
	bool							m_isPanning = false;
	float							m_PanMomentum = 0.0f;

	FVector							m_RotateAnchor;
	bool							m_isRotating = false;

};
