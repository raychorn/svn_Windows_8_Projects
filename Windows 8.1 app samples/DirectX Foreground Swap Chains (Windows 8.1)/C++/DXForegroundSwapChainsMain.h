//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "StepTimer.h"
#include "DeviceResources.h"
#include "DXForegroundSwapChainsRenderer.h"
#include "SampleOverlay.h"

// Renders Direct2D and 3D content on the screen.
namespace DXForegroundSwapChains
{
    class DXForegroundSwapChainsMain : public std::enable_shared_from_this<DXForegroundSwapChainsMain>, public IDeviceLostHandler
    {
    public:
        DXForegroundSwapChainsMain(const std::shared_ptr<DeviceResources>& deviceResources);
        virtual void OnDeviceLost();
        virtual void OnDeviceRestored();
        void UpdateForWindowSizeChange();
        void Update();
        bool Render();

    private:
        // Cached pointer to device resources.
        std::shared_ptr<DeviceResources> m_deviceResources;

        // Sample renderer class.
        std::unique_ptr<DXForegroundSwapChainsRenderer> m_sceneRenderer;

        // Sample overlay class.
        std::unique_ptr<SampleOverlay> m_sampleOverlay;

        // Rendering loop timer.
        DX::StepTimer m_timer;
    };
}