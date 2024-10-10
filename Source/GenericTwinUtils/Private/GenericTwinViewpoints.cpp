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


#include "GenericTwinViewpoints.h"

#include "Common/Configuration/JsonParser.h"

#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogGenericTwinViewpoints);

AGenericTwinViewpoints* AGenericTwinViewpoints::GetGenericTwinViewpoints(UObject* WorldContextObject)
{
	AGenericTwinViewpoints *Viewpoints = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<AActor*> actors;
		UGameplayStatics::GetAllActorsOfClass(World, AGenericTwinViewpoints::StaticClass(), actors);
		const int32 numActors = actors.Num();
		if(numActors > 0)
		{
			Viewpoints = Cast<AGenericTwinViewpoints>(actors[0]);
		}
		else
		{
			UE_LOG(LogGenericTwinViewpoints, Error, TEXT("No GenericTwinViewpoints found"));
		}
	}

	return Viewpoints;
}

bool AGenericTwinViewpoints::Load(const FString &Filename)
{
	bool res = false;
	GenericTwin::FJsonParser parser;

    UE_LOG(LogGenericTwinViewpoints, Log, TEXT("Trying to load %s"), *Filename);

    m_Viewpoints.Empty();

	if(parser.Load(Filename.TrimQuotes()))
	{
        UE_LOG(LogGenericTwinViewpoints, Log, TEXT("Loaded, parsing now"));
		const int32 numWMarkers = parser.PushArray(TEXT("location_markers"));
		for(int32 i = 0; i < numWMarkers; ++i)
		{
			FGenericTwinViewpoint marker;
			if	(	parser.ParseArrayValue	(	i
												,	{   {GenericTwin::EJsonDataType::Double, TEXT("lon"), &marker.Longitude}
													,	{GenericTwin::EJsonDataType::Double, TEXT("lat"), &marker.Latitude}
													,	{GenericTwin::EJsonDataType::Double, TEXT("alt"), &marker.Altitude}
													,	{GenericTwin::EJsonDataType::Float, TEXT("heading"), &marker.Heading}
													,	{GenericTwin::EJsonDataType::Float, TEXT("pitch"), &marker.Pitch}
													,	{GenericTwin::EJsonDataType::String, TEXT("name"), &marker.Name}
				}
											)
				)
			{
				m_Viewpoints.Add( marker );
			}
		}

		parser.PopArray();
		res = true;
	}
    else
    {
        UE_LOG(LogGenericTwinViewpoints, Log, TEXT("Could not load %s"), *Filename);
    }

	m_curIndex = res ? 0 : -1;
	return res;
}

void AGenericTwinViewpoints::NextViewpoint(bool WrapAround, bool &IsValid, FGenericTwinViewpoint &Viewpoint)
{
	IsValid = false;
	if(m_Viewpoints.Num() > 0)
	{
		if(WrapAround)
		{
			m_curIndex = (m_curIndex + 1) % m_Viewpoints.Num();
			Viewpoint = m_Viewpoints[m_curIndex];
			IsValid = true;
		}
		else
		{
			if((m_curIndex + 1) < m_Viewpoints.Num())
			{
				m_curIndex = m_curIndex + 1;
				Viewpoint = m_Viewpoints[m_curIndex];
				IsValid = true;
			}
		}
	}
}

void AGenericTwinViewpoints::PreviousViewpoint(bool WrapAround, bool &IsValid, FGenericTwinViewpoint &Viewpoint)
{
	IsValid = false;
	if(m_Viewpoints.Num() > 0)
	{
		if(WrapAround)
		{
			m_curIndex--;
			if(m_curIndex < 0)
				m_curIndex = m_Viewpoints.Num() - 1;
			Viewpoint = m_Viewpoints[m_curIndex];
			IsValid = true;
		}
		else
		{
			if(m_curIndex > 0)
			{
				m_curIndex--;
				Viewpoint = m_Viewpoints[m_curIndex];
				IsValid = true;
			}
		}
	}
}

void AGenericTwinViewpoints::GetViewpoint(int32 Index, bool &IsValid, FGenericTwinViewpoint &Viewpoint)
{
	if(Index < m_Viewpoints.Num())
	{
		Viewpoint = m_Viewpoints[Index];
		IsValid = true;
	}
}


TArray<FString> AGenericTwinViewpoints::GetViewpoints()
{
	TArray<FString> viewpoints;

	for(const FGenericTwinViewpoint &vp : m_Viewpoints)
	{
		viewpoints.Add(vp.Name);
	}

	return viewpoints;
}
