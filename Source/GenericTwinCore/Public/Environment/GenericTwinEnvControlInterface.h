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
#include "UObject/Interface.h"
#include "GenericTwinEnvControlInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UGenericTwinEnvControlInterface : public UInterface
{
	GENERATED_BODY()

};



/* Actual Interface declaration. */
class IGenericTwinEnvControlInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.

public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GenericTwin|Envrionment")
	void SetLocation(double Longitude, double Latitude);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GenericTwin|Envrionment")
	void SetDate(int32 Day, int32 Month, int32 Year);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GenericTwin|Envrionment")
	void SetSolarTime(float SolarTime);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GenericTwin|Envrionment")
	float GetSolarTime();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GenericTwin|Envrionment")
	TArray<EGenericTwinWeatherType> GetAvailableWeather();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GenericTwin|Envrionment")
	void SetWeather(EGenericTwinWeatherType WeatherType);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="GenericTwin|Envrionment")
	EGenericTwinWeatherType GetWeather();

};
