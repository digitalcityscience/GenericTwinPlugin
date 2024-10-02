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


#include "GenericTwinLocationMarkerList.h"

#include "Common/Configuration/JsonParser.h"

DEFINE_LOG_CATEGORY(LogGenericTwinLocationMarkerList);

bool UGenericTwinLocationMarkerList::Load(const FString &Filename)
{
	bool res = false;
	GenericTwin::FJsonParser parser;

    UE_LOG(LogGenericTwinLocationMarkerList, Log, TEXT("Trying to load %s"), *Filename);


    LocationMarkers.Empty();

	if(parser.Load(Filename.TrimQuotes()))
	{
        UE_LOG(LogGenericTwinLocationMarkerList, Log, TEXT("Loaded, parsing now"));
		const int32 numWMarkers = parser.PushArray(TEXT("location_markers"));
		for(int32 i = 0; i < numWMarkers; ++i)
		{
			FGenericTwinLocationMarker marker;
			if	(	parser.ParseArrayValue	(	i
												,	{   {GenericTwin::EJsonDataType::Double, TEXT("lon"), &marker.Longitude}
													,	{GenericTwin::EJsonDataType::Double, TEXT("lat"), &marker.Latitude}
													,	{GenericTwin::EJsonDataType::Double, TEXT("alt"), &marker.Altitude}
													,	{GenericTwin::EJsonDataType::Float, TEXT("heading"), &marker.Heading}
													,	{GenericTwin::EJsonDataType::Float, TEXT("pitch"), &marker.Pitch}
												}
											)
				)
			{
				LocationMarkers.Add( marker );
			}
		}

		parser.PopArray();
		res = true;
	}
    else
    {
        UE_LOG(LogGenericTwinLocationMarkerList, Log, TEXT("Could not load %s"), *Filename);
    }
	return res;
}

void UGenericTwinLocationMarkerList::NextLocationMarker(bool WrapAround, bool &IsValid, FGenericTwinLocationMarker &LocationMarker)
{
}

void UGenericTwinLocationMarkerList::PreviousLocationMarker(bool WrapAround, bool &IsValid, FGenericTwinLocationMarker &LocationMarker)
{

}

void UGenericTwinLocationMarkerList::SetLocationMarker(int32 Index, bool &IsValid, FGenericTwinLocationMarker &LocationMarker)
{

}


