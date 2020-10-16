//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "DirectXPage.g.h"

#include "DeviceResources.h"
#include "ShadowMappingMain.h"

namespace ShadowMapSample
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
        // XAML low-level rendering event handler.
        void OnRendering(Platform::Object^ sender, Platform::Object^ args);

        // Window event handlers.
        void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
        void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);

        // Display properties event handlers.
        void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ displayInfo, Platform::Object^ sender);
        void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ displayInfo, Platform::Object^ sender);
        void OnDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ displayInfo, Platform::Object^ sender);

        // Other event handlers.
        void OnEnableLinearFiltering(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);
        void OnDisableLinearFiltering(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ args);

        // Resource used to keep track of the rendering event registration.
        Windows::Foundation::EventRegistrationToken m_eventToken;

        // Resources used to render the DirectX content in the XAML page background.
        std::shared_ptr<DeviceResources> m_deviceResources;
        std::shared_ptr<ShadowMapSampleMain> m_main;
        bool m_windowVisible;
    };
}
