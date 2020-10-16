//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "DeviceResources.h"
#include "JpegYCbCrOptimizations.h"

namespace JpegYCbCrOptimizations
{
    // This sample renderer instantiates a basic rendering pipeline.
    class JpegYCbCrOptimizationsRenderer
    {
    public:
        JpegYCbCrOptimizationsRenderer(
            const std::shared_ptr<DeviceResources>& deviceResources,
            _In_ ResourcesLoadedHandler^ handler,
            bool forceBgra
            );

        void Render();
        void CreateWindowSizeDependentResources();
        concurrency::task<void> CancelBackgroundResourceCreationAsync();

    private:
        bool DoesWicSupportRequestedYCbCr();
        bool DoesDriverSupportYCbCr();

        void LockBitmap(
            _In_ IWICBitmap *pBitmap,
            DWORD bitmapLockFlags,
            _In_opt_ const WICRect *prcSource,
            _Outptr_ IWICBitmapLock **ppBitmapLock,
            _Out_ WICBitmapPlane *pPlane
            );

        void CreateDeviceIndependentResources();
        void CreateDeviceDependentResources(bool forceBgra);
        void CreateYCbCrDeviceResources();
        void CreateBgraDeviceResources();
        void GetDisplayResolution();

        // Cached pointer to device resources.
        std::shared_ptr<DeviceResources> m_deviceResources;

        YCbCrSupportMode m_sampleMode;
        uint32 m_cachedBitmapPixelWidth;
        uint32 m_cachedBitmapPixelHeight;
        bool m_isResourceCreationComplete;
        uint32 m_displayResolutionX;
        uint32 m_displayResolutionY;

        // Resources to enable cancelling the background task.
        concurrency::cancellation_token_source m_cancellationTokenSource;
        concurrency::task<void> m_backgroundTask;

        // Called when all resources are ready for rendering.
        ResourcesLoadedHandler^ m_resourcesLoadedHandler;

        Microsoft::WRL::ComPtr<IWICBitmapScaler> m_wicScaler;

        // These resources are only used if YCbCr is enabled.
        WICBitmapPlaneDescription           m_planeDescriptions[SampleConstants::NumPlanes];
        Microsoft::WRL::ComPtr<ID2D1Effect> m_d2dYCbCrEffect;

        // These resources are only used if YCbCr is disabled.
        Microsoft::WRL::ComPtr<ID2D1Effect> m_d2dTransformEffect;
    };
}
