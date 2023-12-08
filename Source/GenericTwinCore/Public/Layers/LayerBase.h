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

#include "LayerTypes.h"

#include "UObject/NoExportTypes.h"
#include "LayerBase.generated.h" 

struct FTwinWorldContext;
class FJsonObject;

enum class ELayerPriority	:	int32
{
	Default			= 0,
	Low				= 10,
	Medium			= 20,
	High			= 30,
	
};

UCLASS()
class GENERICTWINCORE_API ULayerBase	:	public UObject
{
	GENERATED_BODY()

public:

	ULayerBase();
	virtual ~ULayerBase();

	virtual void InitializeLayer(FTwinWorldContext &TwinWorldCtx);

	virtual void UpdateView(FTwinWorldContext &TwinWorldCtx, float DeltaSeconds);

	virtual void OnEndPlay(FTwinWorldContext &TwinWorldCtx);

	virtual void OnVisibilityChanged(FTwinWorldContext &TwinWorldCtx);

	virtual const TSharedPtr<FJsonObject> GetRuntimeConfiguration() const;
	virtual void UpdateConfiguration(TSharedPtr<FJsonObject> UpdatedConfiguration);

	virtual ELayerCoverage GetLayerCoverage(const FBox2d &Area) const;

	const FString& GetLayerType() const;
	ELayerContributionType GetLayerContributionType() const;
	ELayerContentType GetLayerContentType() const;
	const SLayerMetaData& GetLayerMetaData() const;
	const ELayerPriority GetLayerPriority() const;
	void SetVisibility(FTwinWorldContext &TwinWorldCtx, bool NewVisibility);
	bool GetVisibility() const;

	void SetLayerId(int32 LayerId);
	int32 GetLayerId() const;

	void SetCustomName(const FString &CustomName);
	const FString& GetCustomName() const;

	void AddLayerDependency(GenericTwin::LayerDependencyBasePtr &Dependency);

protected:

	void OnConstruction(const SLayerMetaData &MetaData, ELayerPriority Prio);

	SLayerMetaData							m_MetaData;
	ELayerContentType						m_ContentType;
	FString									m_CustomName;

	int32									m_LayerId = 0;

	ELayerPriority							m_LayerPriority;

	bool									m_isVisible = true;

	GenericTwin::LayerDependencyBasePtr		m_Dependency;
};

inline void ULayerBase::OnConstruction(const SLayerMetaData &MetaData, ELayerPriority Prio)
{
	m_MetaData = MetaData;
	m_LayerPriority = Prio;
}


inline const FString& ULayerBase::GetLayerType() const
{
	return m_MetaData.LayerType;
}

inline ELayerContributionType ULayerBase::GetLayerContributionType() const
{
	return m_MetaData.LayerContributionType;
}

inline ELayerContentType ULayerBase::GetLayerContentType() const
{
	return m_ContentType;
}

inline const SLayerMetaData& ULayerBase::GetLayerMetaData() const
{
	return m_MetaData;
}

inline const ELayerPriority ULayerBase::GetLayerPriority() const
{
	return m_LayerPriority;
}

inline bool ULayerBase::GetVisibility() const
{
	return m_isVisible;
}

inline void ULayerBase::SetLayerId(int32 LayerId)
{
	m_LayerId = LayerId;
}

inline int32 ULayerBase::GetLayerId() const
{
	return m_LayerId;
}

inline void ULayerBase::SetCustomName(const FString &CustomName)
{
	m_CustomName = CustomName;
}

inline const FString& ULayerBase::GetCustomName() const
{
	return m_CustomName;
}


inline void ULayerBase::AddLayerDependency(GenericTwin::LayerDependencyBasePtr &Dependency)
{
	m_Dependency = Dependency;
}
