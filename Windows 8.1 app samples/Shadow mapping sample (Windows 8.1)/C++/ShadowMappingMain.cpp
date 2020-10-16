#include "pch.h"
#include "ShadowMappingMain.h"

#include <DirectXColors.h>            // For named colors
#include "Common\DirectXHelper.h"    // For ThrowIfFailed

using namespace ShadowMapSample;

// Loads and initializes application assets when the application is loaded.
ShadowMapSampleMain::ShadowMapSampleMain(const std::shared_ptr<DeviceResources>& deviceResources) :
    m_deviceResources(deviceResources)
{
    m_shadowSceneRenderer = std::unique_ptr<ShadowSceneRenderer>(new ShadowSceneRenderer(m_deviceResources));
}

// Notifies renderers that device resources need to be released.
void ShadowMapSampleMain::OnDeviceLost()
{
    m_shadowSceneRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be re-created.
void ShadowMapSampleMain::OnDeviceRestored()
{
    m_shadowSceneRenderer->CreateDeviceDependentResources();
    UpdateForWindowSizeChange();
}

// Updates application state when the window size changes (e.g. device orientation change)
void ShadowMapSampleMain::UpdateForWindowSizeChange()
{
    m_shadowSceneRenderer->CreateWindowSizeDependentResources();
}

// Updates the application state once per frame.
void ShadowMapSampleMain::Update()
{
    // Update scene objects.
    m_timer.Tick([&]() {
        m_shadowSceneRenderer->Update(m_timer);
    });
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool ShadowMapSampleMain::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return false;
    }

    // Render the scene objects.
    m_shadowSceneRenderer->Render();

    return true;
}

// Set the filtering type for the shadow map.
void ShadowMapSampleMain::SetFiltering(bool useLinear)
{
    m_shadowSceneRenderer->SetFiltering(useLinear);
}
