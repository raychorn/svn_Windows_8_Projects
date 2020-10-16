#pragma once

#include "Common\StepTimer.h"
#include "DeviceResources.h"
#include "Content\ShadowSceneRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace ShadowMapSample
{
    class ShadowMapSampleMain : public std::enable_shared_from_this<ShadowMapSampleMain>, public IDeviceLostHandler
    {
    public:
        ShadowMapSampleMain(const std::shared_ptr<DeviceResources>& deviceResources);
        virtual void OnDeviceLost();
        virtual void OnDeviceRestored();
        void UpdateForWindowSizeChange();
        void Update();
        bool Render();
        void SetFiltering(bool useLinear);

    private:
        // Cached pointer to device resources.
        std::shared_ptr<DeviceResources> m_deviceResources;

        std::unique_ptr<ShadowSceneRenderer> m_shadowSceneRenderer;

        // Rendering loop timer.
        DX::StepTimer m_timer;
    };
}