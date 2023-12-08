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

#include "Common/JobQueue/GenericTwinJobDispatcher.h"
#include "Common/JobQueue/GenericTwinJobBase.h"

// DEFINE_LOG_CATEGORY(LogGenericJobQueue);

FGenericTwinJobDispatcher *FGenericTwinJobDispatcher::s_Instance = 0;

void FGenericTwinJobDispatcher::Shutdown()
{
	if(s_Instance)
	{
		s_Instance->m_BackgroundJobQueue.Stop();
		s_Instance->m_GameThreadJobQueue.Empty();

		delete s_Instance;
		s_Instance = 0;
	}
}

FGenericTwinJobDispatcher::FGenericTwinJobDispatcher()
	:	m_BackgroundJobQueue(TEXT("JobDispatcher"))
{

}

void FGenericTwinJobDispatcher::DispatchJob(const GenericTwin::TJobBasePtr &JobPtr)
{
	if(s_Instance == 0)
	{
		s_Instance = new FGenericTwinJobDispatcher;
	}

	if	(	s_Instance
		&&	JobPtr
		)
	{
		const GenericTwin::JobBase::EJobType jobType = JobPtr->GetJobType();

		switch(jobType)
		{
			case GenericTwin::JobBase::EJobType::Background:
				{
					auto f = std::bind(&FGenericTwinJobDispatcher::ExecuteJob, s_Instance, std::placeholders::_1);
					s_Instance->m_BackgroundJobQueue.QueueRequest(SRequest(JobPtr), f);
				}
				break;

			case GenericTwin::JobBase::EJobType::GameThread:
				s_Instance->m_GameThreadJobQueue.Enqueue(JobPtr);
				break;

			case GenericTwin::JobBase::EJobType::RenderThread:
				// Not implemented yet
				break;

		}
	}
}


FGenericTwinJobDispatcher::SResult FGenericTwinJobDispatcher::ExecuteJob(const SRequest &Request)
{
	return SResult(Request.JobPtr, Request.JobPtr->Execute());
}


void FGenericTwinJobDispatcher::Tick(float DeltaSeconds)
{
	{
		SResult result;
		if(m_BackgroundJobQueue.GetResult(result))
		{
			result.JobPtr->OnJobFinished();
			if(result.NextJobPtr)
				DispatchJob(result.NextJobPtr);
		}
	}

	GenericTwin::TJobBasePtr gtJob;
	if	(	m_GameThreadJobQueue.Dequeue(gtJob)
		&&	gtJob
		)
	{
		GenericTwin::TJobBasePtr nextJob = gtJob->Execute();
		if(nextJob)
			DispatchJob(nextJob);
	}
}

TStatId FGenericTwinJobDispatcher::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FGenericTwinJobDispatcher, STATGROUP_Tickables);
}
