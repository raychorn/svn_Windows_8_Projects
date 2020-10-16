//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// S1_Eventing.xaml.cpp
// Implementation of the S1_Eventing class
//

#include "pch.h"
#include <algorithm>
#include "S1_Eventing.xaml.h"
#include "MainPage.xaml.h"

using namespace std;
using namespace Platform;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

using namespace SDKSample;
using namespace SDKSample::BluetoothGattHeartRate;

S1_Eventing::S1_Eventing()
{
    InitializeComponent();
    this->SizeChanged += ref new Windows::UI::Xaml::SizeChangedEventHandler(
        this, 
        &SDKSample::BluetoothGattHeartRate::S1_Eventing::OnSizeChanged);
}

void S1_Eventing::LoadState(Object^ /*navigationParameter*/, IMap<String^, Object^>^ /*pageState*/)
{
    if (!HeartRateService::Instance->IsServiceInitialized)
    {
        HeartRateService::Instance->InitializeHeartRateServices();
    }

    valueChangeCompletedRegistrationToken = (HeartRateService::Instance->ValueChangeCompleted += 
        ref new ValueChangeCompletedHandler(this, &S1_Eventing::Instance_ValueChangeCompleted));

    for_each (begin(HeartRateService::Instance->DataPoints), end(HeartRateService::Instance->DataPoints), 
        [this] (HeartRateMeasurement^ value)
    {
        outputListBox->Items->Append(value->GetDescription());
    });
}

void S1_Eventing::SaveState(IMap<String^, Object^>^ /*pageState*/)
{
    HeartRateService::Instance->ValueChangeCompleted -= valueChangeCompletedRegistrationToken;
}

void S1_Eventing::Instance_ValueChangeCompleted(HeartRateMeasurement^ heartRateMeasurementValue)
{
    //Serialize UI update to the main UI Thread
    Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, heartRateMeasurementValue] () 
    {
        std::wstringstream wss;
        wss << L"Latest received heart rate measurement: " << heartRateMeasurementValue->HeartRateValue;
        latestHeartRateMeasurementTextBlock->Text = ref new String(wss.str().c_str());

        outputDataChart->PlotChart(HeartRateService::Instance->DataPoints);

        outputListBox->Items->InsertAt(0, heartRateMeasurementValue->GetDescription());
    }));
}

void S1_Eventing::RefreshBluetoothDevices_Click(Object^ sender, RoutedEventArgs^ e)
{
    if (!HeartRateService::Instance->IsServiceInitialized)
    {
        HeartRateService::Instance->InitializeHeartRateServices();
    }
}

void S1_Eventing::OnSizeChanged(Platform::Object^ sender, Windows::UI::Xaml::SizeChangedEventArgs^ e)
{
    outputDataChart->PlotChart(HeartRateService::Instance->DataPoints);
}
