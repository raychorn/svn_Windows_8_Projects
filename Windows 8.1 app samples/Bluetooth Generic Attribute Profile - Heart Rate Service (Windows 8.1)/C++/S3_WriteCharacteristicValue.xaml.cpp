//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// S3_WriteCharacteristicValue.xaml.cpp
// Implementation of the S3_WriteCharacteristicValue class
//

#include "pch.h"
#include "S3_WriteCharacteristicValue.xaml.h"
#include "MainPage.xaml.h"

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;

using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

using namespace SDKSample;
using namespace SDKSample::BluetoothGattHeartRate;

S3_WriteCharacteristicValue::S3_WriteCharacteristicValue()
{
    InitializeComponent();
}

void S3_WriteCharacteristicValue::LoadState(Object^ /*navigationParameter*/, IMap<String^, Object^>^ /*pageState*/)
{
    if (!HeartRateService::Instance->IsServiceInitialized)
    {
        HeartRateService::Instance->InitializeHeartRateServices();
    }

    valueChangeCompletedRegistrationToken = (HeartRateService::Instance->ValueChangeCompleted += 
        ref new ValueChangeCompletedHandler(this, &S3_WriteCharacteristicValue::Instance_ValueChangeCompleted));
}

void S3_WriteCharacteristicValue::SaveState(IMap<String^, Object^>^ /*pageState*/)
{
    HeartRateService::Instance->ValueChangeCompleted -= valueChangeCompletedRegistrationToken;
}

void S3_WriteCharacteristicValue::Instance_ValueChangeCompleted(HeartRateMeasurement^ heartRateMeasurementValue)
{
    //Serialize UI update to the main UI Thread
    Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, heartRateMeasurementValue] () 
    {
        std::wstringstream wss;
        wss << L"Expended Energy: " << heartRateMeasurementValue->ExpendedEnergy << L" kJ";

        ExpendedEnergyTextBlock->Text =  ref new String(wss.str().c_str());
    }));
}

void S3_WriteCharacteristicValue::WriteCharacteristicValue_Click(Object^ sender, RoutedEventArgs^ e)
{
    auto heartRateControlPointCharacteristics = 
        HeartRateService::Instance->Service->GetCharacteristics(GattCharacteristicUuids::HeartRateControlPoint);

    if (heartRateControlPointCharacteristics->Size > 0)
    {
        auto writer = ref new Windows::Storage::Streams::DataWriter();
        writer->WriteByte(1);

        create_task(heartRateControlPointCharacteristics->GetAt(0)->WriteValueAsync(writer->DetachBuffer()))
            .then([this] (GattCommunicationStatus status)
        {
            if (status == GattCommunicationStatus::Success)
            {
                MainPage::Current->NotifyUser("Expended Energy successfully reset.", NotifyType::StatusMessage);
            }
            else
            {
                MainPage::Current->NotifyUser("Your device is unreachable, the device is either out of range, " +
                        "or is running low on battery, please make sure your device is working and try again.",
                        NotifyType::StatusMessage);
            }
        }).then([] (task<void> finalTask)
        {
            try
            {
                finalTask.get();
            }
            catch (COMException^ e)
            {
                MainPage::Current->NotifyUser("Error: " + e->Message, NotifyType::ErrorMessage);
            }
        });
    }
}

void S3_WriteCharacteristicValue::RefreshBluetoothDevices_Click(Object^ sender, RoutedEventArgs^ e)
{
    if (!HeartRateService::Instance->IsServiceInitialized)
    {
        HeartRateService::Instance->InitializeHeartRateServices();
    }
}
