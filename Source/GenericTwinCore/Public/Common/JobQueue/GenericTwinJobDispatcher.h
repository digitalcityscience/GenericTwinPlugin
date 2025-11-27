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

#include "Common/JobQueue/TJobQueue.h"
#include "Common/JobQueue/GenericTwinJobBase.h"
#include "Tickable.h"

namespace GenericTwin {

typedef TSharedPtr<class JobBase> TJobBasePtr;

}	//	namespace GenericTwin 


class GENERICTWINCORE_API FGenericTwinJobDispatcher	:	public	 FTickableGameObject
{
	struct SRequest
	{
		SRequest()
		{
		}

		SRequest(const GenericTwin::TJobBasePtr &_JobPtr)
			:	JobPtr(_JobPtr)
		{
		}

		GenericTwin::TJobBasePtr		JobPtr;
	};

	struct SResult
	{
		SResult()
		{
		}

		SResult(const GenericTwin::TJobBasePtr& _JobPtr, const GenericTwin::TJobBasePtr& _NextJobPtr)
			: JobPtr(_JobPtr)
			, NextJobPtr(_NextJobPtr)
		{
		}

		GenericTwin::TJobBasePtr		JobPtr;
		GenericTwin::TJobBasePtr		NextJobPtr;

	};

	typedef TJobQueue<SRequest, SResult>			BackgroundJobQueue;
	typedef TQueue<GenericTwin::TJobBasePtr>		JobQueue;

public:

	static void Shutdown();

	static void DispatchJob(const GenericTwin::TJobBasePtr &TJobBasePtr);

	template<typename FuncType>
	static void Dispatch(TFunction<FuncType> Function, GenericTwin::EJobType JobType);

	template<typename FuncType>
	static void Dispatch(GenericTwin::EJobType JobType, TFunction<FuncType> Function);


	/*
	*	FTickableGameObject
	*/
	virtual void Tick(float DeltaSeconds) override;

	virtual TStatId GetStatId() const override;


private:

	FGenericTwinJobDispatcher();

	SResult ExecuteJob(const SRequest &Request);

	BackgroundJobQueue						m_BackgroundJobQueue;

	JobQueue								m_GameThreadJobQueue;

	static FGenericTwinJobDispatcher		*s_Instance;

};


template<typename FuncType>
inline void FGenericTwinJobDispatcher::Dispatch(TFunction<FuncType> Function, GenericTwin::EJobType JobType)
{
	Dispatch(JobType, Function);
}

template<typename FuncType>
inline void FGenericTwinJobDispatcher::Dispatch(GenericTwin::EJobType JobType, TFunction<FuncType> Function)
{
	class Job	:	public GenericTwin::JobBase
	{
	public:

		Job(TFunction<FuncType> Function, GenericTwin::EJobType JobType)
			:	JobBase(JobType)
			,	m_Function(Function)
		{
		}

		virtual GenericTwin::TJobBasePtr Execute() override
		{
			m_Function();
			return GenericTwin::TJobBasePtr();
		}

	private:

		TFunction<FuncType>		m_Function;

	};

	TSharedPtr<Job> jobPtr(new Job(Function, JobType));
	FGenericTwinJobDispatcher::DispatchJob(jobPtr);
}
