//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "JpegYCbCrOptimizationsMain.h"

#include <DirectXColors.h> // For named colors
#include "DirectXHelper.h" // For ThrowIfFailed

using namespace JpegYCbCrOptimizations;
using namespace concurrency;

// Loads and initializes application assets when the application is loaded.
JpegYCbCrOptimizationsMain::JpegYCbCrOptimizationsMain(
    const std::shared_ptr<DeviceResources>& deviceResources,
    _In_ ResourcesLoadedHandler^ handler,
    bool isBgraForced
    ) : m_deviceResources(deviceResources),
        m_isForcedBgraMode(false),
        m_resourcesLoadedHandler(handler)
{
    m_isForcedBgraMode = isBgraForced;
    CreateRenderer();
}

/// <summary>
/// This method returns immediately but the renderer may be created asynchronously.
/// </summary>
void JpegYCbCrOptimizationsMain::CreateRenderer()
{
    if (m_sceneRenderer != nullptr)
    {
        // Ensure the renderer's background resource loading task has terminated before destroying it.
        m_sceneRenderer->CancelBackgroundResourceCreationAsync().then([this]()
        {
            m_sceneRenderer = std::unique_ptr<JpegYCbCrOptimizationsRenderer>(
                new JpegYCbCrOptimizationsRenderer(m_deviceResources, m_resourcesLoadedHandler, m_isForcedBgraMode)
                );

        // m_sceneRenderer must be constructed on the UI thread.
        }, task_continuation_context::use_current());
    }
    else
    {
        m_sceneRenderer = std::unique_ptr<JpegYCbCrOptimizationsRenderer>(
            new JpegYCbCrOptimizationsRenderer(m_deviceResources, m_resourcesLoadedHandler, m_isForcedBgraMode)
            );
    }
}

void JpegYCbCrOptimizationsMain::SetIsBgraForced(bool isBgraForced)
{
    // Only recreate the renderer if the mode has changed.
    if (isBgraForced != m_isForcedBgraMode)
    {
        m_isForcedBgraMode = isBgraForced;
        CreateRenderer();
    }
}

// Notifies renderers that device resources need to be released.
void JpegYCbCrOptimizationsMain::OnDeviceLost()
{
    if (m_sceneRenderer != nullptr)
    {
        // Ensure the renderer's background resource loading task has terminated before destroying it.
        m_sceneRenderer->CancelBackgroundResourceCreationAsync().then([this]()
        {
            m_sceneRenderer = nullptr;
        });
    }
}

// Notifies renderers that device resources may now be re-created.
void JpegYCbCrOptimizationsMain::OnDeviceRestored()
{
    CreateRenderer();
}

// Updates application state when the window size changes (e.g. device orientation change)
void JpegYCbCrOptimizationsMain::UpdateForWindowSizeChange()
{
    m_sceneRenderer->CreateWindowSizeDependentResources();
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool JpegYCbCrOptimizationsMain::Render()
{
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Reset the viewport to target the whole screen.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // Reset render targets to the screen.
    ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

    // Clear the back buffer and depth stencil view.
    context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::MidnightBlue);
    context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Render the scene objects.
    m_sceneRenderer->Render();

    return true;
}
