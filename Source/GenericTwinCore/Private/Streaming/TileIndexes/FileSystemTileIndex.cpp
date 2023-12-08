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

#include "Streaming/TileIndexes/FileSystemTileIndex.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Math/NumericLimits.h"

#include "GeoReferencingSystem.h"

DEFINE_LOG_CATEGORY(LogFileSystemTileIndex);

namespace GenericTwin {

FileSystemTileIndex::FileSystemTileIndex(AGeoReferencingSystem &GeoReferencingSystem)
	:	m_GeoReferencingSystem(GeoReferencingSystem)
{
}

bool FileSystemTileIndex::Initialize(const FString& TileStreamingFile, TParseParamsFuncPtr ParseParamsFunc)
{
	bool res = false;

	FString jsonStr;
	if (FFileHelper::LoadFileToString(jsonStr, *TileStreamingFile))
	{
		UE_LOG(LogFileSystemTileIndex, Log, TEXT("Successfully opened tile description %s"), *TileStreamingFile);
		TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonStr);

		TSharedPtr<FJsonObject> jsonObject;
		if (FJsonSerializer::Deserialize(reader, jsonObject))
		{
			UE_LOG(LogFileSystemTileIndex, Log, TEXT("Successfully deserialized material mapping config file"));

			if (	jsonObject->HasTypedField<EJson::Number>(TEXT("tile_size"))
				&&	jsonObject->HasTypedField<EJson::String>(TEXT("base_url"))
				&&	jsonObject->HasTypedField<EJson::Array>(TEXT("tiles"))
				)
			{
				m_TileSize.X = m_TileSize.Y = jsonObject->GetNumberField(TEXT("tile_size"));
				FString baseUrl = GenericTwin::FPathUtils::MakeAbsolutPath(jsonObject->GetStringField(TEXT("base_url")), FPaths::LaunchDir());

				const TArray < TSharedPtr < FJsonValue > >* tilesArray = 0;
				if (jsonObject->TryGetArrayField(TEXT("tiles"), tilesArray)
					&& tilesArray
					)
				{
					TArray<STile> tiles;
					double minTileX = TNumericLimits< double >::Max();
					double minTileY = TNumericLimits< double >::Max();

					for (const auto& tileVal : (*tilesArray))
					{
						if (tileVal->Type == EJson::Object)
						{
							const TSharedPtr<FJsonObject> &tileObj = tileVal->AsObject();
							if (	tileObj->HasTypedField<EJson::Number>(TEXT("tile_x"))
								&&	tileObj->HasTypedField<EJson::Number>(TEXT("tile_y"))
								&&	tileObj->HasTypedField<EJson::String>(TEXT("tile_url"))
								)
							{
								STile tile;
								tile.Origin.X = tileObj->GetNumberField(TEXT("tile_x"));
								tile.Origin.Y = tileObj->GetNumberField(TEXT("tile_y"));
								tile.Url = tileObj->GetStringField(TEXT("tile_url"));
								tiles.Add(tile);

								int32 tileX = static_cast<int32> (tile.Origin.X / m_TileSize.X);
								int32 tileY = static_cast<int32> (tile.Origin.Y / m_TileSize.Y);

								if(tile.Origin.X < minTileX)
									minTileX = tile.Origin.X;
								if(tile.Origin.Y < minTileY)
									minTileY = tile.Origin.Y;

							}
						}
					}

					if(tiles.Num() > 0)
					{
						for(const STile &tile : tiles)
						{
							const int32 x = static_cast<int32> ((tile.Origin.X - minTileX) / m_TileSize.X);
							const int32 y = static_cast<int32> ((tile.Origin.Y - minTileY) / m_TileSize.Y);
							const TTileId tileId(x, y);

							m_AvailableTiles.Add(tileId, STile(tile.Origin, FPaths::Combine(baseUrl, tile.Url)));
						}

						m_GeoReferencingSystem.ProjectedToEngine(FVector(minTileX, minTileY, 0.0), m_ReferenceLocation);

						m_ReferenceTileIndex.X = static_cast<int32> (minTileX / m_TileSize.X);
						m_ReferenceTileIndex.Y = static_cast<int32> (minTileY / m_TileSize.Y);
						m_TileSize *= 100.0;		// transform into Unreal units

						UE_LOG(LogFileSystemTileIndex, Log, TEXT("Build tile index containing %d tiles"), m_AvailableTiles.Num() );

						if(jsonObject->HasTypedField<EJson::Object>(TEXT("parameters")))
						{
							const TSharedPtr<FJsonObject> paramsObject = jsonObject->GetObjectField(TEXT("parameters"));
							ParseParamsFunc(*(paramsObject.Get()));
						}

						res = true;
					}
				}
			}
		}
		else
		{
			UE_LOG(LogFileSystemTileIndex, Error, TEXT("Not a valid JSON file") );
		}
	}
	else
	{
		UE_LOG(LogFileSystemTileIndex, Error, TEXT("File %s not found"), *TileStreamingFile);
	}

	return res;
}

FVector FileSystemTileIndex::GetReferenceLocation() const
{
	return m_ReferenceLocation;
}

FVector2D FileSystemTileIndex::GetTileSize() const
{
	return m_TileSize;
}

bool FileSystemTileIndex::HasTile(const TTileId &TileId) const
{
	return m_AvailableTiles.Contains(TileId);
}

FVector2D FileSystemTileIndex::GetTileOrigin(const TTileId &TileId) const
{
	return m_AvailableTiles.Contains(TileId) ? m_AvailableTiles[TileId].Origin : FVector2D();
}

FString FileSystemTileIndex::GetUrl(const TTileId &TileId) const
{
	return m_AvailableTiles.Contains(TileId) ? m_AvailableTiles[TileId].Url : FString();
}

}	//	namespace GenericTwin
