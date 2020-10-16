//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// Scenario4.xaml.cpp
// Implementation of the Scenario4 class
//

#include "pch.h"
#include "Scenario4.xaml.h"
#include "MainPage.xaml.h"

using namespace SDKSample;
using namespace SDKSample::RequestedThemeCPP;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

Scenario4::Scenario4()
{
	InitializeComponent();
}

void RequestedThemeCPP::Scenario4::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	RevertRequestedTheme(panel);
}


void RequestedThemeCPP::Scenario4::RevertRequestedTheme(FrameworkElement^ fe)
{
	if (fe->RequestedTheme == ElementTheme::Dark)
		fe->RequestedTheme = ElementTheme::Light;
	else
		fe->RequestedTheme = ElementTheme::Dark;
	CurrentThemeTxtBlock->Text = "Current theme is " + fe->RequestedTheme.ToString() + ".";
}


