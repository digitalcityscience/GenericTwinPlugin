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

#include "Windows/AllowWindowsPlatformTypes.h"

#define UI UI_ST
THIRD_PARTY_INCLUDES_START

#define CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_NO_EXCEPTIONS

#include "httplib.h"

THIRD_PARTY_INCLUDES_END
#undef UI

#include "Windows/HideWindowsPlatformTypes.h"


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

		auto f = std::bind(&FGenericTwinHttpRequestManager::PerformRequest, this, std::placeholders::_1);
		m_RequestQueue.QueueRequest(reqData, f);
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

TSharedPtr<FGenericTwinHttpRequestManager::SResponseData> FGenericTwinHttpRequestManager::PerformRequest(const SRequestData &RequestData)
{
	TSharedPtr<SResponseData> responsePtr(new SResponseData);

	if(responsePtr)
	{
		responsePtr->RequestData = RequestData;
		responsePtr->Success = false;

		httplib::Client cli(TCHAR_TO_ANSI(*RequestData.Domain));
		switch(RequestData.Method)
		{
			if(RequestData.TimeoutS > 0)
				cli.set_connection_timeout(std::chrono::seconds(RequestData.TimeoutS));
			else
				cli.set_connection_timeout(std::chrono::seconds(30));

			case ERequestMethod::GET:
				{
					UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Sending GET request to %s <-> %s"), *RequestData.Domain, *RequestData.Path);

					auto httpResponse = cli.Get(TCHAR_TO_ANSI(*RequestData.Path));


					if (httpResponse)
					{
						UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Response received %d"), httpResponse->status);
						responsePtr->ResponseCode = httpResponse->status;
						responsePtr->StatusMessage = FString(httpResponse->reason.c_str());
						if(httpResponse->status == 200)
						{
							responsePtr->Response = FString(httpResponse->body.c_str());
							responsePtr->Success = true;
						}
						else if (httpResponse->status == 301)
						{
							UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Permanently moved: %s"), *(FString(httpResponse->body.c_str())));
						}
						else if (httpResponse->status == 400)
						{
							UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Bad request: %s"), *(FString(httpResponse->body.c_str())));
						}
					}
					else
					{
						UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Request failed"));
					}
				}
				break;

			case ERequestMethod::POST:
				{
					UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Sending POST request to %s <-> %s"), *RequestData.Domain, *RequestData.Path);

					httplib::Headers headers;
					std::string contentType("application/x-www-form-urlencoded");
					auto httpResponse = cli.Post(TCHAR_TO_ANSI(*RequestData.Path), headers, TCHAR_TO_ANSI(*RequestData.Body), contentType);

					if (httpResponse)
					{
						UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Response received %d"), httpResponse->status);
						responsePtr->ResponseCode = httpResponse->status;
						responsePtr->StatusMessage = FString(httpResponse->reason.c_str());
						if(httpResponse->status == 200)
						{
							responsePtr->Response = FString(httpResponse->body.c_str());
							responsePtr->Success = true;
						}
						else if (httpResponse->status == 301)
						{
							UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Permanently moved: %s"), *(FString(httpResponse->body.c_str())));
						}
						else if (httpResponse->status == 400)
						{
							UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Bad request: %s"), *(FString(httpResponse->body.c_str())));
						}
					}
					else
					{
						UE_LOG(LogGenericTwinHttpRequestManager, Log, TEXT("Request failed"));
					}
				}
				break;
		}
	}

	return responsePtr;
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
