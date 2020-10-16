//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "StepTimer.h"
#include "DeviceResources.h"
#include "TerrainRenderer.h"
#include "SamplingRenderer.h"
#include "ResidencyManager.h"
#include "TileLoader.h"
#include "SampleOverlay.h"

// Renders Direct2D and 3D content on the screen.
namespace TiledResources
{
    class TiledResourcesMain : public std::enable_shared_from_this<TiledResourcesMain>, public IDeviceLostHandler
    {
    public:
        TiledResourcesMain(const std::shared_ptr<DeviceResources>& deviceResources);
        virtual void OnDeviceLost();
        virtual void OnDeviceRestored();
        void UpdateForWindowSizeChange();
        void Update();
        bool Render();
        void OnPointerMoved(float dx, float dy);
        void OnKeyChanged(Windows::System::VirtualKey key, bool down);
        void OnRightClick(float x, float y);

    private:
        concurrency::task<void> CreateDeviceDependentResourcesAsync();

        // Cached pointer to device resources.
        std::shared_ptr<DeviceResources> m_deviceResources;

        std::unique_ptr<TerrainRenderer> m_terrainRenderer;
        std::unique_ptr<SamplingRenderer> m_samplingRenderer;
        std::unique_ptr<ResidencyManager> m_residencyManager;

        // Sample overlay class.
        std::unique_ptr<SampleOverlay> m_sampleOverlay;

        // Rendering loop timer.
        DX::StepTimer m_timer;

        // Camera controller.
        FreeCamera m_camera;

        // Camera control state.
        struct
        {
            bool vxp; bool vxn; // +/- X velocity
            bool vyp; bool vyn; // +/- Y velocity
            bool vzp; bool vzn; // +/- Z velocity
            float vrx; // X rotation (pitch) is transient
            float vry; // Y rotation (yaw) is transient
            bool vrzp; bool vrzn; // +/- Z rotation (roll)
        } m_controlState;

        bool m_renderersLoaded;

        bool m_debugMode;
    };
}
