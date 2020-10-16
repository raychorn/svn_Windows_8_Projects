//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "DirectXPage.g.h"

#include "DeviceResources.h"
#include "JpegYCbCrOptimizationsMain.h"
#include "JpegYCbCrOptimizations.h"

namespace JpegYCbCrOptimizations
{
    /// <summary>
    /// A page that hosts a DirectX SwapChainBackgroundPanel.
    /// This page must be the root of the Window content (it cannot be hosted on a Frame).
    /// </summary>
    public ref class DirectXPage sealed
    {
    public:
        DirectXPage();

        void SaveInternalState(Windows::Foundation::Collections::IPropertySet^ state);
        void LoadInternalState(Windows::Foundation::Collections::IPropertySet^ state);

    private:
        void RenderDXContent();

        // Called when JpegYCbCrOptimizationsRenderer has finished creating resources.
        void OnResourcesLoaded(YCbCrSupportMode mode);

        // Window event handlers.
        void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
        void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

        // Display properties event handlers.
        void OnDpiChanged(_In_ Windows::Graphics::Display::DisplayInformation^ sender, _In_ Platform::Object^ args);
        void OnOrientationChanged(_In_ Windows::Graphics::Display::DisplayInformation^ sender, _In_ Platform::Object^ args);
        void OnDisplayContentsInvalidated(_In_ Windows::Graphics::Display::DisplayInformation^ sender, _In_ Platform::Object^ args);

        // Button click handlers.
        void OnForceBgraClicked(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

        // Resource used to keep track of the rendering event registration.
        Windows::Foundation::EventRegistrationToken m_eventToken;

        // Resources used to render the DirectX content in the XAML page background.
        std::shared_ptr<DeviceResources> m_deviceResources;
        std::shared_ptr<JpegYCbCrOptimizationsMain> m_main;

        bool m_windowVisible;
    };
}
