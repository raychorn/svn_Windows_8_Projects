//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "StepTimer.h"
#include "DeviceResources.h"
#include "D2DGeometryRealizationsRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace D2DGeometryRealizations
{
    class D2DGeometryRealizationsMain : public std::enable_shared_from_this<D2DGeometryRealizationsMain>, public IDeviceLostHandler
    {
    public:
        D2DGeometryRealizationsMain(const std::shared_ptr<DeviceResources>& deviceResources);
        virtual void OnDeviceLost();
        virtual void OnDeviceRestored();
        void UpdateForWindowSizeChange();
        void Update();
        bool Render();

        unsigned int GetFPS();

        void UpdateZoom(Windows::Foundation::Point position, Windows::Foundation::Point positionDelta, float zoomDelta);

        void IncreasePrimitives();
        void DecreasePrimitives();

        bool GetRealizationsEnabled();
        void SetRealizationsEnabled(bool enabled);

        void RestoreDefaults();

        void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
        void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

    private:
        void SetViewPosition(Windows::Foundation::Point viewPosition);
        void SetZoom(float zoom);

        void UpdateWorldMatrix();

        // Cached pointer to device resources.
        std::shared_ptr<DeviceResources> m_deviceResources;

        // Sample renderer class.
        std::unique_ptr<D2DGeometryRealizationsRenderer> m_sceneRenderer;

        // Rendering loop timer.
        DX::StepTimer m_timer;

        // Position of the viewing rectangle on the scene.
        Windows::Foundation::Point m_viewPosition;

        // The scale factor at which the scene is rendered.
        float m_zoom;
    };
}