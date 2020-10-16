//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#include "pch.h"
#include "MainPage.xaml.h"
#include "Constants.h"

using namespace SDKSample;

Platform::Array<Scenario>^ MainPage::scenariosInner = ref new Platform::Array<Scenario>  
{
    // The format here is the following:
    //     { "Description for the sample", "Fully qualified name for the class that implements the scenario" }
    { "Select & Initialize", "SDKSample.UsbCdcControl.CdcAcmInitialize" },
    { "Read Data", "SDKSample.UsbCdcControl.CdcAcmRead" },
    { "Write Data", "SDKSample.UsbCdcControl.CdcAcmWrite" },
    { "Loopback Test", "SDKSample.UsbCdcControl.CdcAcmLoopback" },
}; 