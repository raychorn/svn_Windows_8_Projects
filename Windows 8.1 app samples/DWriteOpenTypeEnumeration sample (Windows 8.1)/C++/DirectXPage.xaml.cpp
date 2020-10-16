//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace DWriteOpenTypeEnumeration;

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

    DisplayInformation::GetForCurrentView()->DpiChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

    DisplayInformation::GetForCurrentView()->OrientationChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

    DisplayInformation::DisplayContentsInvalidated +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

    // Register the rendering event, called every time XAML renders the screen.
    m_eventToken = CompositionTarget::Rendering::add(ref new EventHandler<Object^>(this, &DirectXPage::OnRendering));

    // Disable all pointer visual feedback for better performance when touching.
    auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
    pointerVisualizationSettings->IsContactFeedbackEnabled = false;
    pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;

    // At this point we have access to the device.
    // We can create the device-dependent resources.
    m_deviceResources = std::make_shared<DeviceResources>();
    m_deviceResources->SetWindow(Window::Current->CoreWindow, swapChainBackgroundPanel);

    m_main = std::shared_ptr<DWriteOpenTypeEnumerationMain>(new DWriteOpenTypeEnumerationMain(m_deviceResources));
    m_deviceResources->RegisterDeviceLostHandler(m_main->shared_from_this());
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
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

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
    m_deviceResources->SetDpi(sender->LogicalDpi);
    m_main->UpdateForWindowSizeChange();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
    m_deviceResources->UpdateForWindowSizeChange();
    m_main->UpdateForWindowSizeChange();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
    m_deviceResources->ValidateDevice();
}

void DirectXPage::OnFontSelectionChanged(Object^ sender, SelectionChangedEventArgs^ e)
{
    ComboBox^ cb = safe_cast<ComboBox^>(sender);
    Platform::Array<bool>^ temp = m_main->ReturnSupportedFeatures(cb->SelectedIndex);
    ResetCheckBoxes();

    if (temp[0] == true)
    {
        style0->Visibility = ::Visibility::Visible;
    }
    if (temp[1] == true)
    {
        style1->Visibility = ::Visibility::Visible;
    }
    if (temp[2] == true)
    {
        style2->Visibility = ::Visibility::Visible;
    }
    if (temp[3] == true)
    {
        style3->Visibility = ::Visibility::Visible;
    }
    if (temp[4] == true)
    {
        style4->Visibility = ::Visibility::Visible;
    }
    if (temp[5] == true)
    {
        style5->Visibility = ::Visibility::Visible;
    }
    if (temp[6] == true)
    {
        style6->Visibility = ::Visibility::Visible;
    }
    if (temp[7] == true)
    {
        style7->Visibility = ::Visibility::Visible;
    }
    if (!temp[1] && !temp[2] && !temp[3] && !temp[4] && !temp[5] && !temp[6] && !temp[7])
    {
        stylisticSet->Visibility = ::Visibility::Collapsed;
        stylisticSetLabel->Text = "No Stylistic Sets Present in this Font";
    }
}

void DirectXPage::ResetCheckBoxes()
{
    stylisticSet->Visibility = ::Visibility::Visible;
    style0->Visibility = ::Visibility::Collapsed;
    style1->Visibility = ::Visibility::Collapsed;
    style2->Visibility = ::Visibility::Collapsed;
    style3->Visibility = ::Visibility::Collapsed;
    style4->Visibility = ::Visibility::Collapsed;
    style5->Visibility = ::Visibility::Collapsed;
    style6->Visibility = ::Visibility::Collapsed;
    style7->Visibility = ::Visibility::Collapsed;
    stylisticSet->SelectedIndex = 8;
    stylisticSetLabel->Text = "Stylistic Set";
}

void DirectXPage::OnStylisticSetSelectionChanged(Object^ sender, SelectionChangedEventArgs^ e)
{
    m_main->SetStylisticSet(safe_cast<ComboBox^>(sender)->SelectedIndex);
}