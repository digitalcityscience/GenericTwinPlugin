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
#include "UObject/NoExportTypes.h"
#include "GenericTwinLocationMarkerList.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGenericTwinLocationMarkerList, Log, All);

USTRUCT(BlueprintType)
struct FGenericTwinLocationMarker
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = Default)
	FString	Name;

	UPROPERTY(BlueprintReadOnly, Category = Default)
	double	Longitude;

	UPROPERTY(BlueprintReadOnly, Category = Default)
	double	Latitude;

	UPROPERTY(BlueprintReadOnly, Category = Default)
	double	Altitude;

	UPROPERTY(BlueprintReadOnly, Category = Default)
	float	Heading;

	UPROPERTY(BlueprintReadOnly, Category = Default)
	float	Pitch;

};


/**
 * 
 */
UCLASS(BlueprintType)
class GENERICTWINUTILS_API UGenericTwinLocationMarkerList : public UObject
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Utils|LocationMarker")
	bool Load(const FString &Filename);

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Utils|LocationMarker")
	void NextLocationMarker(bool WrapAround, bool &IsValid, FGenericTwinLocationMarker &LocationMarker);

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Utils|LocationMarker")
	void PreviousLocationMarker(bool WrapAround, bool &IsValid, FGenericTwinLocationMarker &LocationMarker);

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Utils|LocationMarker")
	void SetLocationMarker(int32 Index, bool &IsValid, FGenericTwinLocationMarker &LocationMarker);

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Utils|LocationMarker")
	int32 GetLocationMarkerCount();

private:

	TArray<FGenericTwinLocationMarker>	LocationMarkers;

};

inline int32 UGenericTwinLocationMarkerList::GetLocationMarkerCount()
{
	return LocationMarkers.Num();
}
