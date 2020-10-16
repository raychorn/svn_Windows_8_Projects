//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "JpegYCbCrOptimizations.h"

// Constants and sample-specific data.
namespace JpegYCbCrOptimizations
{
    namespace SampleConstants
    {
        // The Direct2D YCbCr effect requires DXGI_FORMAT_R8_UNORM and DXGI_FORMAT_R8G8_UNORM,
        // which determines the requested WIC YCbCr configuration.
        const WICPixelFormatGUID WicYCbCrFormats[NumPlanes] =
        {
            GUID_WICPixelFormat8bppY,
            GUID_WICPixelFormat16bppCbCr
        };

        Platform::String^ SampleModeString_DisabledFallback(L": YCbCr disabled (fallback)");
        Platform::String^ SampleModeString_DisabledForced(L": YCbCr disabled (forced)");
        Platform::String^ SampleModeString_Enabled(L": YCbCr enabled");
        Platform::String^ TitleString(L"JPEG YCbCr Optimizations");
    }
}