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

DECLARE_LOG_CATEGORY_EXTERN(LogJsonParser, Log, All);

namespace GenericTwin
{

typedef TSharedPtr<FJsonObject> JsonObjectPtr;

enum class EJsonDataType : uint8
{
	Undefined,
	String,
	Bool,
	Integer,
	Float,
	Double,
	Vector2,
	Vector3,
	Vector4,
	Color,
	Enumerator

};

struct FJsonObjectMember
{
	EJsonDataType			Type;
	FString					Key;
	void					*Destination;
	bool					IsOptional = false;

	TMap<FString, uint32>	EnumMapping;
};

class GENERICTWINCORE_API FJsonParser
{

	typedef TArray< TSharedPtr<FJsonValue> >	Array;

public:

	bool Load(const FString &JsonFileName);

	bool Parse(const TSharedPtr<FJsonObject> &RootObjPtr, const TArray<FJsonObjectMember> &Members);

	bool Parse(const TArray<FJsonObjectMember> &Members);

	bool ParseObject(const JsonObjectPtr &JsonObject, const TArray<FJsonObjectMember> &Members);

	int32 PushArray(const FString &Key);
	int32 PushArray(int32 Index, const FString &Key);
	bool ParseArrayValue(int32 Index, const TArray<FJsonObjectMember> &Members);
	bool ParseArrayObject(int32 Index, const FString &ObjectKey, const TArray<FJsonObjectMember> &Members);

	void PopArray();



	void Pop();

private:

	bool CheckType(EJsonDataType DesiredType, EJson ValueType);

	TArray<JsonObjectPtr>			m_ObjectStack;

	TArray< Array >					m_ArrayStack;

};

}	//	namespace GenericTwin
