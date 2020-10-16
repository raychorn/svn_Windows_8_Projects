// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//
// Scenario3.xaml.cpp
// Implementation of the Scenario3 class
//

#include "pch.h"
#include "Scenario3.xaml.h"
#include "MainPage.xaml.h"

using namespace SDKSample;
using namespace SDKSample::Flyouts;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

Scenario3::Scenario3()
{
    InitializeComponent();
}

void Flyouts::Scenario3::textBlock_Tapped(Platform::Object^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs^ e)
{
    TextBlock^ tb = dynamic_cast<TextBlock^>(sender);
    if (tb != nullptr)
    {
		Flyout::ShowAttachedFlyout(tb);

        MainPage::Current->NotifyUser("You opened a Flyout", NotifyType::StatusMessage);
    }
}
