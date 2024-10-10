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
#include "GameFramework/Info.h"
#include "GenericTwinViewpoints.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGenericTwinViewpoints, Log, All);

USTRUCT(BlueprintType)
struct FGenericTwinViewpoint
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
class GENERICTWINUTILS_API AGenericTwinViewpoints : public AInfo
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "GenericTwin|Utils|Viewpoints", meta = (WorldContext = "WorldContextObject"))
	static AGenericTwinViewpoints* GetGenericTwinViewpoints(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Utils|Viewpoints")
	bool Load(const FString &Filename);

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Utils|Viewpoints")
	void NextViewpoint(bool WrapAround, bool &IsValid, FGenericTwinViewpoint &Viewpoint);

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Utils|Viewpoints")
	void PreviousViewpoint(bool WrapAround, bool &IsValid, FGenericTwinViewpoint &Viewpoint);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GenericTwin|Utils|Viewpoints")
	void GetViewpoint(int32 Index, bool &IsValid, FGenericTwinViewpoint &Viewpoint);

	UFUNCTION(BlueprintCallable, Category="GenericTwin|Utils|Viewpoints")
	int32 GetViewpointCount();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GenericTwin|Utils|Viewpoints")
	TArray<FString> GetViewpoints();

private:

	TArray<FGenericTwinViewpoint>			m_Viewpoints;
	int32									m_curIndex = -1;

};

inline int32 AGenericTwinViewpoints::GetViewpointCount()
{
	return m_Viewpoints.Num();
}
