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

#include "Tickable.h"

#include "Common/JobQueue/TJobQueue.h"


DECLARE_LOG_CATEGORY_EXTERN(LogGenericTwinHttpRequestManager, Log, All);


DECLARE_DELEGATE_TwoParams(FOnGenericTwinHttpResponse, int32, const FString&);
DECLARE_DELEGATE_ThreeParams(FOnGenericTwinHttpRequestError, int32, int32, const FString&);

class FHttpModule;

class GENERICTWINCORE_API FGenericTwinHttpRequestManager	:	public FTickableGameObject
{
public:

	enum ERequestMethod
	{
		GET,
		POST
	};

private:

	struct SRequestData
	{
		int32									RequestId = 0;

		FString									Domain;
		FString									Path;
		ERequestMethod							Method;
	
		TArray<TPair<FString, FString>>			Headers;

		int32									TimeoutS = -1.0;		

		FOnGenericTwinHttpResponse				OnResponse;
		FOnGenericTwinHttpRequestError			OnError;

	};

	struct SResponseData
	{
		bool						Success = false;
		int32						ResponseCode = 0;
		FString						StatusMessage;

		FString						Response;

		SRequestData				RequestData;
	};

	typedef TJobQueue<SRequestData, TSharedPtr<SResponseData>>	TJobQueue;

public:

	static FGenericTwinHttpRequestManager& GetInstance();

	static void Shutdown();

	int32 CreateRequest(const FString &Url, ERequestMethod RequestMethod, const TArray<FString> &Headers = TArray<FString>());

	int32 CreateRequest(const FString &Domain, const FString &Path, ERequestMethod RequestMethod, const TArray<FString> &Headers = TArray<FString>());

	bool SendRequest(int32 RequestId, FOnGenericTwinHttpResponse OnResponse, FOnGenericTwinHttpRequestError OnError);

	void CancelRequest(int32 RequestId);

	void AddHeader(int32 RequestId, const FString &Key, const FString &Value);

	void SetTimeout(int32 RequestId, int32 TimeoutInS);

	static bool SplitUrl(const FString &Url, FString &Domain, FString &Path);

	/*
	SaveResult(RequestId, Filename, OnResult, OnError )
	*/

	/*
	*	FTickableGameObject
	*/
	virtual void Tick(float DeltaSeconds) override;

	virtual TStatId GetStatId() const override;

private:

	FGenericTwinHttpRequestManager();

	virtual ~FGenericTwinHttpRequestManager();

	TSharedPtr<SResponseData> PerformRequest(const SRequestData &RequestData);

	static FGenericTwinHttpRequestManager			*s_Instance;

	TJobQueue										m_RequestQueue;

	TMap<int32, SRequestData>						m_Requests;
	int32											m_nextRequestId = 1;

};

