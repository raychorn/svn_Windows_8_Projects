//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#include "pch.h"
#include "MainPage.xaml.h"
#include "Constants.h"

using namespace SDKSample;
//using namespace SDKSample::Application1;
using namespace Windows::Networking::Connectivity;

Platform::Array<Scenario>^ MainPage::scenariosInner = ref new Platform::Array<Scenario>  
{
    // The format here is the following:
    //     { "Description for the sample", "Fully qualified name for the class that implements the scenario" }
    { "Create Connection", "SDKSample.Application1.S1_CreateConnection" }, 
    { "Add RoutePolicy", "SDKSample.Application1.S2_AddRoutePolicy" }
}; 


