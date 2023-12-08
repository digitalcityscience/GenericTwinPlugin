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

#include "Layers/Dependencies/LayerCoverageDependency.h"
#include "Layers/LayerBase.h"

#include "Dom/JsonObject.h"

namespace GenericTwin
{

bool LayerCoverageDependency::Configure(const FJsonObject &ConfigurationObject)
{
	m_RequiredCoverage = ELayerCoverage::Undefined;
	if(ConfigurationObject.HasTypedField<EJson::String>(TEXT("coverage")))
	{
		const FString coverageStr = ConfigurationObject.GetStringField(TEXT("coverage"));

		if(coverageStr.Compare(TEXT("NotCovered"), ESearchCase::IgnoreCase) == 0)
			m_RequiredCoverage = ELayerCoverage::NotCovered;
		else if(coverageStr.Compare(TEXT("potentially"), ESearchCase::IgnoreCase) == 0)
			m_RequiredCoverage = ELayerCoverage::PotentionallyCovered;
		else if(coverageStr.Compare(TEXT("partly"), ESearchCase::IgnoreCase) == 0)
			m_RequiredCoverage = ELayerCoverage::PartlyCovered;
		else if(coverageStr.Compare(TEXT("fully"), ESearchCase::IgnoreCase) == 0)
			m_RequiredCoverage = ELayerCoverage::FullyCovered;
	}

	return m_RequiredCoverage != ELayerCoverage::Undefined;
}

bool LayerCoverageDependency::IsDependencyMet(const FBox2d &Area) const
{
	for(const ULayerBase *layer : GetLayers())
	{
		if(layer->GetLayerCoverage(Area) >= m_RequiredCoverage)
			return true;
	}

	return false;
}

}	//	namespace GenericTwin
