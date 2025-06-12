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


#include "Common/Communication/GenericTwinHttpRequestManager.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"


DEFINE_LOG_CATEGORY(LogGenericTwinHttpRequestManager);


FGenericTwinHttpRequestManager* FGenericTwinHttpRequestManager::s_Instance = 0;

FGenericTwinHttpRequestManager& FGenericTwinHttpRequestManager::GetInstance()
{
	if(s_Instance == 0)
	{
		s_Instance = new FGenericTwinHttpRequestManager;
	}

	return *s_Instance;
}

void FGenericTwinHttpRequestManager::Shutdown()
{
	if(s_Instance)
	{
		s_Instance->m_RequestQueue.Stop();
		delete s_Instance;
	}
}


FGenericTwinHttpRequestManager::FGenericTwinHttpRequestManager()
	:	m_RequestQueue(TEXT("GenericTwinHttpRequestManager"))
{
}

FGenericTwinHttpRequestManager::~FGenericTwinHttpRequestManager()
{
}

int32 FGenericTwinHttpRequestManager::CreateRequest(const FString &Url, ERequestMethod RequestMethod, const TArray<FString> &Headers)
{
	return CreateRequest(Url, FString(), RequestMethod, Headers);
}

int32 FGenericTwinHttpRequestManager::CreateRequest(const FString &Domain, const FString &Path, ERequestMethod RequestMethod, const TArray<FString> &Headers)
{
	int32 reqId = m_nextRequestId++;

	SRequestData request;

	request.RequestId = reqId;
	request.Domain = Domain;
	request.Path = Path;
	request.Method = RequestMethod;

	m_Requests.Add(reqId, request);

	return reqId;
}

int32 FGenericTwinHttpRequestManager::CreateRequest(const FString &Domain, const FString &Path, ERequestMethod RequestMethod, const TArray<TPair<FString, FString>> &Headers, const TArray<TPair<FString, FString>> &Parameters)
{
	int32 reqId = m_nextRequestId++;

	SRequestData request;

	request.RequestId = reqId;
	request.Domain = Domain;
	request.Path = Parameters.IsEmpty() ? Path : EncodeUrl(Path, Parameters);
	request.Method = RequestMethod;

	for(const auto &item : Headers)
	{
		request.Headers.Add({item.Key, item.Value});
	}


	m_Requests.Add(reqId, request);

	return reqId;
}

int32 FGenericTwinHttpRequestManager::CreatePostRequestWithBody(const FString &Domain, const FString &Path, const FString &Body, const TArray<TPair<FString, FString>> &Headers)
{
	int32 reqId = m_nextRequestId++;

	SRequestData request;
	request.RequestId = reqId;
	request.Domain = Domain;
	request.Path = Path;
	request.Method = ERequestMethod::POST;
	request.Body = Body;

	for(const auto &item : Headers)
	{
		request.Headers.Add({item.Key, item.Value});
	}

	m_Requests.Add(reqId, request);

	return reqId;
}

void FGenericTwinHttpRequestManager::SetTimeout(int32 RequestId, int32 TimeoutInS)
{
	if(m_Requests.Contains(RequestId))
		m_Requests[RequestId].TimeoutS = TimeoutInS;
}

bool FGenericTwinHttpRequestManager::SendRequest(int32 RequestId, FOnGenericTwinHttpResponse OnResponse, FOnGenericTwinHttpRequestError OnError)
{
	bool res = false;

	if(m_Requests.Contains(RequestId))
	{
		SRequestData &reqData = m_Requests[RequestId];

		reqData.OnResponse = OnResponse;
		reqData.OnError = OnError;

		ProcessRequest(reqData);
	}

	return res;
}

void FGenericTwinHttpRequestManager::Tick(float DeltaSeconds)
{
	TSharedPtr<SResponseData> responsePtr;
	if	(	m_RequestQueue.GetResult(responsePtr)
		&&	responsePtr
		)
	{
		if(responsePtr->Success)
		{
			responsePtr->RequestData.OnResponse.ExecuteIfBound(responsePtr->RequestData.RequestId, responsePtr->Response);
		}
		else
		{
			responsePtr->RequestData.OnError.ExecuteIfBound(responsePtr->RequestData.RequestId, responsePtr->ResponseCode, responsePtr->Response);
		}

		m_Requests.Remove(responsePtr->RequestData.RequestId);
	}
}

void FGenericTwinHttpRequestManager::ProcessRequest(SRequestData RequestData)
{
	FHttpModule &httpModule = FHttpModule::Get();
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> request = httpModule.CreateRequest();
	switch(RequestData.Method)
	{
		case ERequestMethod::GET:
			request->SetVerb(TEXT("GET"));
			request->SetURL(RequestData.Domain + RequestData.Path);
			break;

		case ERequestMethod::POST:
			request->SetVerb(TEXT("POST"));
			request->SetURL(RequestData.Domain + RequestData.Path);
			request->SetContentAsString(RequestData.Body);
			request->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
			break;
	}

	for( const auto &item : RequestData.Headers)
	{
		request->SetHeader(item.Key, item.Value);
	}

	request->OnProcessRequestComplete().BindLambda(
		[this, RequestData](
			FHttpRequestPtr Request,
			FHttpResponsePtr Response,
			bool connectedSuccessfully) mutable {

			const bool validResponse = Response.IsValid();
			if	(	connectedSuccessfully
				&&	validResponse
				)
			{
				const int32 responseCode = Response->GetResponseCode();
				if(responseCode == 200)
				{
					RequestData.OnResponse.ExecuteIfBound(RequestData.RequestId, Response->GetContentAsString());
				}
				else
				{
					if (responseCode == 301)
					{
						UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Permanently moved: %s"), *(Response->GetContentAsString()));
					}
					else if (responseCode == 400)
					{
						UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Bad request: %s"), *(Response->GetContentAsString()));
					}

					RequestData.OnError.ExecuteIfBound(RequestData.RequestId, responseCode, Response->GetContentAsString());
				}
			}
			else
			{
				if(validResponse)
				{
					switch (Request->GetStatus())
					{
						case EHttpRequestStatus::Failed_ConnectionError:
							UE_LOG(LogGenericTwinHttpRequestManager, Error, TEXT("Connection failed."));
						default:
							UE_LOG(LogGenericTwinHttpRequestManager, Error, TEXT("Request failed."));
					}
					RequestData.OnError.ExecuteIfBound(RequestData.RequestId, Response->GetResponseCode(), Response->GetContentAsString());
				}
				else
				{
					RequestData.OnError.ExecuteIfBound(RequestData.RequestId, 0, TEXT("Time out"));
				}
			}
	});

	request->ProcessRequest();

}

TStatId FGenericTwinHttpRequestManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FGenericTwinHttpRequestManager, STATGROUP_Tickables);
}

void FGenericTwinHttpRequestManager::CancelRequest(int32 RequestId)
{
}

void FGenericTwinHttpRequestManager::AddHeader(int32 RequestId, const FString &Key, const FString &Value)
{
	if(m_Requests.Contains(RequestId))
	{
		m_Requests[RequestId].Headers.Add({Key, Value});
	}
}


bool FGenericTwinHttpRequestManager::SplitUrl(const FString &Url, FString &Domain, FString &Path)
{
	bool res = false;
	TArray<FString> parts;
	// if(Url.Split(parts, TEXT("/")) > 0)
	{

	}

	return res;
}

FString FGenericTwinHttpRequestManager::EncodeUrl(const FString &BaseUrl, const TArray<TPair<FString, FString>> &Parameters)
{
	FString encodedUrl(BaseUrl);

	TCHAR sep = '?';
	for(const auto &item : Parameters)
	{
		FString encoded;

		for (const auto &c : item.Value)
		{
			if	(	c == TCHAR(':')
				|| c == TCHAR('/')
				)
			{
				char buf[16];
				sprintf_s(buf, sizeof(buf), "%02X", static_cast<uint16_t> (c) );

				encoded += TCHAR('%');
				encoded += ANSI_TO_TCHAR(buf);
			}
			else
			{
				encoded += c;
			}
			// if (std::isalnum(c)) {
			// 	escaped << c;
			// } else if (c == '-') {
			// 	escaped << c;
			// } else if (c == '_') {
			// 	escaped << c;
			// } else if (c == '.') {
			// 	escaped << c;
			// } else if (c == '~') {
			// 	escaped << c;
			// } else if (c == '/') {
			// 	escaped << c;
			// } else {
			// 	// Encode as %XX where XX is the hex value of the character
			// 	escaped << '%' << std::setw(2) << int((unsigned char)c);
			// }

		}

		encodedUrl += sep;
		encodedUrl += item.Key;
		encodedUrl += TCHAR('=');
		encodedUrl += encoded;

		sep = TCHAR('&');
	}

	return encodedUrl;
}
