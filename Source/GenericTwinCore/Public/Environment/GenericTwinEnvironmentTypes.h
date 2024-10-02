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


UENUM(BlueprintType)
enum class EGenericTwinWeatherType : uint8
{	
	UNDEFINED				UMETA(DisplayName = "Undefined"),
	CLEAR_SKY				UMETA(DisplayName = "Clear Sky"),
	PARTLY_CLOUDY			UMETA(DisplayName = "Partly Cloudy"),
	CLOUDY					UMETA(DisplayName = "Cloudy"),
	OVERCAST				UMETA(DisplayName = "Overcast"),
	FOGGY					UMETA(DisplayName = "Foggy"),
	LIGHT_RAIN				UMETA(DisplayName = "Light Rain"),
	RAIN					UMETA(DisplayName = "Rain"),
	THUNDERSTORM			UMETA(DisplayName = "Thunderstorm"),
	LIGHT_SNOW				UMETA(DisplayName = "Light Snow"),
	SNOW					UMETA(DisplayName = "Snow"),
	BLIZZARD				UMETA(DisplayName = "Blizzard"),
	LIGHT_SANDSTORM			UMETA(DisplayName = "Light Sand Storm"),
	SANDSTORM				UMETA(DisplayName = "Sand Storm"),
};

