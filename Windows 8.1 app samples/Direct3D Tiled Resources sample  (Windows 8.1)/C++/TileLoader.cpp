//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "DirectXHelper.h"
#include "SampleSettings.h"
#include "ResidencyManager.h"
#include "TileLoader.h"

using namespace TiledResources;

using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::ApplicationModel;

using namespace concurrency;

TileLoader::TileLoader(const std::wstring & filename, std::vector<D3D11_SUBRESOURCE_TILING>* tilingInfo) :
    m_filename(filename),
    m_tilingInfo(tilingInfo)
{
    m_openStreamTask = create_task(Package::Current->InstalledLocation->GetFileAsync(Platform::StringReference(filename.c_str()))).then([this](StorageFile^ file)
    {
        return file->OpenReadAsync();
    }).then([this](IRandomAccessStream^ stream)
    {
        return stream;
    });

    size_t offset = 0;
    for (size_t i = 0; i < m_tilingInfo->size(); i++)
    {
        m_subresourceTileOffsets.push_back(offset);
        size_t numTilesInFileForSubresource = 1;
        if (m_tilingInfo->at(i).StartTileIndexInOverallResource != D3D11_PACKED_TILE)
        {
            numTilesInFileForSubresource = m_tilingInfo->at(i).WidthInTiles * m_tilingInfo->at(i).HeightInTiles;
        }
        offset += numTilesInFileForSubresource;
    }
}

task<std::vector<byte>> TileLoader::LoadTileAsync(D3D11_TILED_RESOURCE_COORDINATE coordinate)
{
    size_t fileOffset = (m_subresourceTileOffsets[coordinate.Subresource] + (coordinate.Y * m_tilingInfo->at(coordinate.Subresource).WidthInTiles + coordinate.X)) * SampleSettings::TileSizeInBytes;
    return m_openStreamTask.then([this, fileOffset](IRandomAccessStream^ stream)
    {
        DX::ThrowIfFailed(fileOffset <= stream->Size - SampleSettings::TileSizeInBytes ? S_OK : E_UNEXPECTED);
        auto reader = ref new DataReader(stream->GetInputStreamAt(fileOffset));
        return create_task(reader->LoadAsync(SampleSettings::TileSizeInBytes)).then([this, reader](unsigned int bytesRead)
        {
            DX::ThrowIfFailed(bytesRead == SampleSettings::TileSizeInBytes ? S_OK : E_UNEXPECTED);
            std::vector<byte> tileData(SampleSettings::TileSizeInBytes);
            reader->ReadBytes(Platform::ArrayReference<byte>(tileData.data(), SampleSettings::TileSizeInBytes));
            return tileData;
        });
    });
}
