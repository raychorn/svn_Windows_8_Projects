//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "DeviceResources.h"
#include "DWriteVerticalHelloWorldRenderer.h"
#include "SampleOverlay.h"

// Renders Direct2D and 3D content on the screen.
namespace DWriteVerticalHelloWorld
{
    class DWriteVerticalHelloWorldMain : public std::enable_shared_from_this<DWriteVerticalHelloWorldMain>, public IDeviceLostHandler
    {
    public:
        DWriteVerticalHelloWorldMain(const std::shared_ptr<DeviceResources>& deviceResources);
        virtual void OnDeviceLost();
        virtual void OnDeviceRestored();
        void UpdateForWindowSizeChange();
        bool Render();

    private:
        // Cached pointer to device resources.
        std::shared_ptr<DeviceResources> m_deviceResources;

        // Sample renderer class.
        std::unique_ptr<DWriteVerticalHelloWorldRenderer> m_sceneRenderer;

        // Sample overlay class.
        std::unique_ptr<SampleOverlay> m_sampleOverlay;
    };
}