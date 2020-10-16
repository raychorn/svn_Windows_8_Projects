//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// S4_Revoke.xaml.cpp
// Implementation of the S4_Revoke class
//

#include "pch.h"
#include "S4_Revoke.xaml.h"
#include "MainPage.xaml.h"

using namespace SDKSample;
using namespace SDKSample::FileRevocation;

using namespace concurrency;
using namespace Platform;
using namespace Windows::Storage;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Security::EnterpriseData;

S4_Revoke::S4_Revoke()
{
    InitializeComponent();
    RootPage = MainPage::Current;
}

void FileRevocation::S4_Revoke::Revoke_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if ("" == InputTextBox->Text)
    {
        RootPage->NotifyUser("Please enter an Enterpise ID that you want to use.", NotifyType::ErrorMessage);
        return;
    }

    FileRevocationManager::Revoke(InputTextBox->Text);

    RootPage->NotifyUser("The Enterprise ID " + InputTextBox->Text + " was revoked. The files protected by it will not be accessible anymore.", NotifyType::StatusMessage);
}
