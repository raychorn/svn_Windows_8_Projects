//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace ShadowMapSample;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

DirectXPage::DirectXPage():
    m_windowVisible(true)
{
    InitializeComponent();

    // Register event handlers for page lifecycle.
    CoreWindow^ window = Window::Current->CoreWindow;

    window->SizeChanged +=
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &DirectXPage::OnWindowSizeChanged);

    window->VisibilityChanged +=
        ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

    auto displayInfo = DisplayInformation::GetForCurrentView();

    displayInfo->DpiChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

    displayInfo->OrientationChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

    displayInfo->DisplayContentsInvalidated +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

    // Register the rendering event, called every time XAML renders the screen.
    m_eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &DirectXPage::OnRendering));

    // Whenever this screen is not being used anymore, you can unregister this event with the following line:
    // CompositionTarget::Rendering::remove(m_eventToken);

    // Disable all pointer visual feedback for better performance when touching.
    auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
    pointerVisualizationSettings->IsContactFeedbackEnabled = false;
    pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;

    // At this point we have access to the device.
    // We can create the device-dependent resources.
    m_deviceResources = std::make_shared<DeviceResources>();
    m_deviceResources->SetWindow(Window::Current->CoreWindow, swapChainBackgroundPanel);

    m_main = std::shared_ptr<ShadowMapSampleMain>(new ShadowMapSampleMain(m_deviceResources));
    m_deviceResources->RegisterDeviceLostHandler(m_main->shared_from_this());
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
    // DirectX apps call Trim() to reduce memory footprint before suspend.
    m_deviceResources->Trim();
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
}

// Called every time XAML decides to render a frame.
void DirectXPage::OnRendering(Object^ sender, Object^ args)
{
    if (m_windowVisible)
    {
        m_main->Update();

        if (m_main->Render())
        {
            m_deviceResources->Present();
        }
    }
}

// Window event handlers.

void DirectXPage::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
    m_deviceResources->UpdateForWindowSizeChange();
    m_main->UpdateForWindowSizeChange();
}

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    m_windowVisible = args->Visible;
}

// Display properties event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ displayInfo, Object^ sender)
{
    m_deviceResources->SetDpi(displayInfo->LogicalDpi);
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ displayInfo, Object^ sender)
{
    m_deviceResources->UpdateForWindowSizeChange();
    m_main->UpdateForWindowSizeChange();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ displayInfo, Object^ sender)
{
    m_deviceResources->ValidateDevice();
}

void DirectXPage::OnEnableLinearFiltering(Object^ sender, RoutedEventArgs^ args)
{
    if (m_main != nullptr)
    {
        m_main->SetFiltering(true);
    }
}

void DirectXPage::OnDisableLinearFiltering(Object^ sender, RoutedEventArgs^ args)
{
    if (m_main != nullptr)
    {
        m_main->SetFiltering(false);
    }
}
