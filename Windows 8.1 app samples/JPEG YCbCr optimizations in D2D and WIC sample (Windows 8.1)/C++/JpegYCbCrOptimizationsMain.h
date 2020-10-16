//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "DeviceResources.h"
#include "JpegYCbCrOptimizationsRenderer.h"
#include "JpegYCbCrOptimizations.h"

// Renders Direct2D and 3D content on the screen.
namespace JpegYCbCrOptimizations
{
    class JpegYCbCrOptimizationsMain : public std::enable_shared_from_this<JpegYCbCrOptimizationsMain>, public IDeviceLostHandler
    {
    public:
        JpegYCbCrOptimizationsMain(
            const std::shared_ptr<DeviceResources>& deviceResources,
            _In_ ResourcesLoadedHandler^ handler,
            bool isBgraForced
            );

        virtual void OnDeviceLost();
        virtual void OnDeviceRestored();
        void UpdateForWindowSizeChange();
        bool Render();

        bool GetIsBgraForced() { return m_isForcedBgraMode; }
        void SetIsBgraForced(bool isBgraForced);

    private:
        void CreateRenderer();

        // Handler is provided by DirectXPage and cached here.
        ResourcesLoadedHandler^ m_resourcesLoadedHandler;

        // Cached pointer to device resources.
        std::shared_ptr<DeviceResources> m_deviceResources;

        // Sample renderer class.
        std::unique_ptr<JpegYCbCrOptimizationsRenderer> m_sceneRenderer;

        // Indicates whether the renderer should use BGRA resources regardless
        // of whether the YCbCr configuration is supported.
        bool m_isForcedBgraMode;
    };
}