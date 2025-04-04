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


#include "Common/Configuration/JsonParser.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY(LogJsonParser);

namespace GenericTwin
{

FJsonParser::FJsonParser()
{
}

FJsonParser::FJsonParser(const FString &JsonString)
{
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(JsonString);
	JsonObjectPtr rootObjPtr;
	if (FJsonSerializer::Deserialize(reader, rootObjPtr))
	{
		UE_LOG(LogJsonParser, Log, TEXT("Successfully deserialized json string"));
		m_ObjectStack.Add(rootObjPtr);
	}
	else
	{
		UE_LOG(LogJsonParser, Error, TEXT("Not a valid json string"));
	}
}

FJsonParser::FJsonParser(const TSharedPtr<FJsonObject> &RootObjPtr)
{
	m_ObjectStack.Add(RootObjPtr);
}

bool FJsonParser::Load(const FString &JsonFileName)
{
	bool res = false;

	FString jsonStr;
	if (FFileHelper::LoadFileToString(jsonStr, *JsonFileName))
	{
		UE_LOG(LogJsonParser, Log, TEXT("Successfully opened file"));
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonStr);

		JsonObjectPtr rootObjPtr;
		if (FJsonSerializer::Deserialize(reader, rootObjPtr))
		{
			UE_LOG(LogJsonParser, Log, TEXT("Successfully deserialized json config file"));
			m_ObjectStack.Add(rootObjPtr);
			res = true;
		}
		else
			UE_LOG(LogJsonParser, Error, TEXT("Not a valid json file"));
	}

	return res;
}

bool FJsonParser::Parse(const TSharedPtr<FJsonObject> &RootObjPtr, const TArray<FJsonObjectMember> &Members)
{
	m_ObjectStack.Add(RootObjPtr);
	return ParseObject(m_ObjectStack.Last(), Members);
}

bool FJsonParser::Parse(const TArray<FJsonObjectMember> &Members)
{
	return ParseObject(m_ObjectStack.Last(), Members);
}

bool FJsonParser::ParseObject(const JsonObjectPtr &JsonObject, const TArray<FJsonObjectMember> &Members)
{
	bool res = false;

	for(const FJsonObjectMember &member : Members)
	{
		bool curRes = false;
		if(JsonObject->Values.Contains(member.Key))
		{
			switch(member.Type)
			{
				case EJsonDataType::String:
					if(JsonObject->Values[member.Key]->Type == EJson::String)
					{
						FString &dst = *(static_cast<FString*>(member.Destination));
						dst = JsonObject->Values[member.Key]->AsString();
						curRes = true;
					}
					break;

				case EJsonDataType::Enumerator:
					if(JsonObject->Values[member.Key]->Type == EJson::String)
					{
						FString valStr = JsonObject->Values[member.Key]->AsString();
						if(member.EnumMapping.Contains(valStr))
						{
							uint32 *dst = static_cast<uint32*>(member.Destination);
							*dst = member.EnumMapping[valStr];
							curRes = true;
						}
					}
					break;

				case EJsonDataType::Bool:
					if(JsonObject->Values[member.Key]->Type == EJson::Boolean)
					{
						bool *dst = static_cast<bool*>(member.Destination);
						*dst = JsonObject->Values[member.Key]->AsBool();
						curRes = true;
					}
					break;

				case EJsonDataType::Integer:
					if(JsonObject->Values[member.Key]->Type == EJson::Number)
					{
						int32 *dst = static_cast<int32*>(member.Destination);
						*dst = static_cast<int32>(JsonObject->Values[member.Key]->AsNumber());
						curRes = true;
					}
					break;

				case EJsonDataType::Integer64:
					if(JsonObject->Values[member.Key]->Type == EJson::Number)
					{
						int64 *dst = static_cast<int64*>(member.Destination);
						*dst = static_cast<int64>(JsonObject->Values[member.Key]->AsNumber());
						curRes = true;
					}
					break;

				case EJsonDataType::Float:
					if(JsonObject->Values[member.Key]->Type == EJson::Number)
					{
						float *dst = static_cast<float*>(member.Destination);
						*dst = static_cast<float>(JsonObject->Values[member.Key]->AsNumber());
						curRes = true;
					}
					break;

				case EJsonDataType::Double:
					if(JsonObject->Values[member.Key]->Type == EJson::Number)
					{
						double *dst = static_cast<double*>(member.Destination);
						*dst = JsonObject->Values[member.Key]->AsNumber();
						curRes = true;
					}
					break;

				case EJsonDataType::Vector2:
					if(JsonObject->Values[member.Key]->Type == EJson::Array)
					{
						const TArray< TSharedPtr< FJsonValue > > &array = JsonObject->Values[member.Key]->AsArray();
						if	(	array.Num() == 2
							&&	array[0]->Type == EJson::Number
							&&	array[1]->Type == EJson::Number
							)
						{
							FVector2D *dst = static_cast<FVector2D*>(member.Destination);
							*dst = FVector2D(array[0]->AsNumber(), array[1]->AsNumber());
							curRes = true;
						}
					}
					break;

				case EJsonDataType::Vector3:
					if(JsonObject->Values[member.Key]->Type == EJson::Array)
					{
						const TArray< TSharedPtr< FJsonValue > > &array = JsonObject->Values[member.Key]->AsArray();
						if	(	array.Num() == 3
							&&	array[0]->Type == EJson::Number
							&&	array[1]->Type == EJson::Number
							&&	array[2]->Type == EJson::Number
							)
						{
							FVector *dst = static_cast<FVector*>(member.Destination);
							*dst = FVector(array[0]->AsNumber(), array[1]->AsNumber(), array[2]->AsNumber());
							curRes = true;
						}
					}
					break;

				case EJsonDataType::Vector4:
					if(JsonObject->Values[member.Key]->Type == EJson::Array)
					{
						const TArray< TSharedPtr< FJsonValue > > &array = JsonObject->Values[member.Key]->AsArray();
						if	(	array.Num() == 4
							&&	array[0]->Type == EJson::Number
							&&	array[1]->Type == EJson::Number
							&&	array[2]->Type == EJson::Number
							&&	array[3]->Type == EJson::Number
							)
						{
							FVector4 *dst = static_cast<FVector4*>(member.Destination);
							*dst = FVector4(array[0]->AsNumber(), array[1]->AsNumber(), array[2]->AsNumber(), array[3]->AsNumber());
							curRes = true;
						}
					}
					break;

				case EJsonDataType::Rotator:
					if(JsonObject->Values[member.Key]->Type == EJson::Array)
					{
						const TArray< TSharedPtr< FJsonValue > > &array = JsonObject->Values[member.Key]->AsArray();
						if	(	array.Num() == 3
							&&	array[0]->Type == EJson::Number
							&&	array[1]->Type == EJson::Number
							&&	array[2]->Type == EJson::Number
							)
						{
							FRotator *dst = static_cast<FRotator*>(member.Destination);
							*dst = FRotator(array[0]->AsNumber(), array[1]->AsNumber(), array[2]->AsNumber());
							curRes = true;
						}
					}
					break;


				case EJsonDataType::Color:
					if(JsonObject->Values[member.Key]->Type == EJson::Array)
					{
						const TArray< TSharedPtr< FJsonValue > > &array = JsonObject->Values[member.Key]->AsArray();
						if	(	array.Num() == 2
							&&	array[0]->Type == EJson::Number
							&&	array[1]->Type == EJson::Number
							&&	array[2]->Type == EJson::Number
							&&	array[3]->Type == EJson::Number
							)
						{
							FColor *dst = static_cast<FColor*>(member.Destination);
							*dst = FColor(array[0]->AsNumber(), array[1]->AsNumber(), array[2]->AsNumber(), array[3]->AsNumber());
							curRes = true;
						}
					}
					break;

			}
		}

		res = curRes || member.IsOptional;
		if(res == false)
		{
			break;
		}
	}

	return res;
}

int32 FJsonParser::PushArray(const FString &Key)
{
	int32 res = -1;

	if(m_ObjectStack.Num())
	{
		JsonObjectPtr &objPtr = m_ObjectStack.Last();
		if	(	objPtr->Values.Contains(Key)
			&&	objPtr->Values[Key]->Type == EJson::Array
			)
		{
			m_ArrayStack.Add(objPtr->Values[Key]->AsArray());
			res = objPtr->Values[Key]->AsArray().Num();
		}
	}

	return res;
}

int32 FJsonParser::PushArray(int32 Index, const FString &Key)
{
	int32 res = -1;

	if(m_ArrayStack.Num())
	{
		Array &curArray = m_ArrayStack.Last();
		if	(	Index < curArray.Num()
			&&	curArray[Index]->Type == EJson::Object
			)
		{
			const JsonObjectPtr &objPtr = curArray[Index]->AsObject();
			if	(	objPtr->Values.Contains(Key)
				&&	objPtr->Values[Key]->Type == EJson::Array
				)
			{
				m_ArrayStack.Add(objPtr->Values[Key]->AsArray());
				res = objPtr->Values[Key]->AsArray().Num();
			}
		}
	}

	return res;
}

bool FJsonParser::ParseArrayValue(int32 Index, const TArray<FJsonObjectMember> &Members)
{
	bool res = false;

	if(m_ArrayStack.Num() > 0)
	{
		Array &curArray = m_ArrayStack.Last();
		if	(	Index < curArray.Num()
			&&	curArray[Index]->Type == EJson::Object
			)
		{
			res = ParseObject(curArray[Index]->AsObject(), Members);
		}
	}


	return res;
}

bool FJsonParser::ParseArrayObject(int32 Index, const FString &ObjectKey, const TArray<FJsonObjectMember> &Members)
{
	bool res = false;

	if(m_ArrayStack.Num() > 0)
	{
		Array &curArray = m_ArrayStack.Last();
		if	(	Index < curArray.Num()
			&&	curArray[Index]->Type == EJson::Object
			)
		{
			const JsonObjectPtr objectPtr = curArray[Index]->AsObject();
			if(objectPtr->HasTypedField<EJson::Object>(ObjectKey))
			{
				res = ParseObject(objectPtr->GetObjectField(ObjectKey), Members);
			}
		}
	}
	return res;
}

void FJsonParser::PopArray()
{
	if(m_ArrayStack.Num() > 0)
		m_ArrayStack.RemoveAt( m_ArrayStack.Num() - 1 );
}

void FJsonParser::Pop()
{

}


bool FJsonParser::CheckType(EJsonDataType DesiredType, EJson ValueType)
{
	switch(DesiredType)
	{
		case EJsonDataType::String:
		case EJsonDataType::Enumerator:
			return ValueType == EJson::String;
		case EJsonDataType::Bool:
			return ValueType == EJson::Boolean;
		case EJsonDataType::Integer:
		case EJsonDataType::Integer64:
		case EJsonDataType::Float:
		case EJsonDataType::Double:
			return ValueType == EJson::Number;
		case EJsonDataType::Vector2:
		case EJsonDataType::Vector3:
		case EJsonDataType::Vector4:
		case EJsonDataType::Rotator:
		case EJsonDataType::Color:
			return ValueType == EJson::Array;
	}
	return false;
}

}	//	namespace GenericTwin
