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

#include "HAL/Runnable.h"
#include "HAL/ThreadingBase.h"

#include "Containers/Queue.h"

#include <functional>

DECLARE_LOG_CATEGORY_EXTERN(LogGenericJobQueue, Log, All);

template<typename REQ_TYPE, typename RES_TYPE>
class TJobQueue	:	public FRunnable
{

public:

	typedef std::function< RES_TYPE(const REQ_TYPE&) > ProcessFuncPtr;

private:

	struct SRequestData
	{
		REQ_TYPE		Request;
		ProcessFuncPtr	Process;		
	};


	typedef TQueue<SRequestData>	RequestQueue;
	typedef TQueue<RES_TYPE>		ResponseQueue;

public:

	TJobQueue(const FString &_Name = FString());

	virtual ~TJobQueue();

	void QueueRequest(const REQ_TYPE &Request);

	void QueueRequest(const REQ_TYPE &Request, ProcessFuncPtr ProcessFunc);

	bool GetResult(RES_TYPE &Result);

	virtual RES_TYPE Process(const REQ_TYPE& )
	{	return RES_TYPE();	}

private:
	/** FRunnable **/
	virtual bool Init();
	virtual uint32 Run();
	virtual void Exit();

public:
	virtual void Stop();


private:

	FString					m_Name;

	ProcessFuncPtr			m_GlobalProcessFunction;

	RequestQueue			m_RequestQueue;
	ResponseQueue			m_ResponseQueue;

	FRunnableThread			*m_WorkerThread = 0;

	bool					m_isRunning = false;

};

template<typename REQ_TYPE, typename RES_TYPE>
inline TJobQueue<REQ_TYPE, RES_TYPE>::TJobQueue(const FString &_Name)
	:	m_Name(_Name)
	,	m_RequestQueue()
	,	m_ResponseQueue()
{
	m_WorkerThread = FRunnableThread::Create(this, *(m_Name + TEXT("_Thread")), 0, TPri_Normal);
}

template<typename REQ_TYPE, typename RES_TYPE>
inline TJobQueue<REQ_TYPE, RES_TYPE>::~TJobQueue()
{
	// cleanUp();
	delete m_WorkerThread;
}

template<typename REQ_TYPE, typename RES_TYPE>
inline void TJobQueue<REQ_TYPE, RES_TYPE>::QueueRequest(const REQ_TYPE &Request)
{
	m_RequestQueue.Enqueue({Request, m_GlobalProcessFunction});
}

template<typename REQ_TYPE, typename RES_TYPE>
inline void TJobQueue<REQ_TYPE, RES_TYPE>::QueueRequest(const REQ_TYPE &Request, ProcessFuncPtr ProcessFunc)
{
	m_RequestQueue.Enqueue({Request, ProcessFunc});
}

template<typename REQ_TYPE, typename RES_TYPE>
inline bool TJobQueue<REQ_TYPE, RES_TYPE>::GetResult(RES_TYPE &Result)
{
	return m_ResponseQueue.Dequeue(Result);
}

template<typename REQ_TYPE, typename RES_TYPE>
inline bool TJobQueue<REQ_TYPE, RES_TYPE>::Init()
{
	m_isRunning = true;

	// UE_LOG(LogGenericJobQueue, Log, TEXT("%s initialized"), *m_Name );

	return true;
}

template<typename REQ_TYPE, typename RES_TYPE>
inline uint32 TJobQueue<REQ_TYPE, RES_TYPE>::Run()
{
	// UE_LOG(LogGenericJobQueue, Log, TEXT("%s started"), *m_Name );
	while(m_isRunning)
	{
		SRequestData requestData;
		if(m_RequestQueue.Dequeue(requestData))
		{

			m_ResponseQueue.Enqueue( requestData.Process(requestData.Request) );
		}

		FPlatformProcess::SleepNoStats(0.02f);
	}

	// UE_LOG(LogGenericJobQueue, Log, TEXT("%s stopped"), *m_Name );

	//cleanUp();
	return 0;
}

template<typename REQ_TYPE, typename RES_TYPE>
inline void TJobQueue<REQ_TYPE, RES_TYPE>::Stop()
{
	// UE_LOG(LogGenericJobQueue, Log, TEXT("Trying to stop %s"), *m_Name );
	m_isRunning = false;
	if(m_WorkerThread)
		m_WorkerThread->WaitForCompletion();
}

template<typename REQ_TYPE, typename RES_TYPE>
inline void TJobQueue<REQ_TYPE, RES_TYPE>::Exit()
{
}

