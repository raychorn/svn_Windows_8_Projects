//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// S2_ReadCharacteristicValue.xaml.cpp
// Implementation of the S2_ReadCharacteristicValue class
//

#include "pch.h"
#include "S2_ReadCharacteristicValue.xaml.h"
#include "MainPage.xaml.h"

using namespace concurrency;
using namespace Platform;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;

using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

using namespace SDKSample;
using namespace SDKSample::BluetoothGattHeartRate;

S2_ReadCharacteristicValue::S2_ReadCharacteristicValue()
{
    InitializeComponent();
}

void BluetoothGattHeartRate::S2_ReadCharacteristicValue::ReadValueButton_Click(
    Platform::Object^ sender, 
    Windows::UI::Xaml::RoutedEventArgs^ e)
{
    auto bodySensorLocationCharacteristics = 
        HeartRateService::Instance->Service->GetCharacteristics(GattCharacteristicUuids::BodySensorLocation);

    if (bodySensorLocationCharacteristics->Size > 0)
    {
        // Read the characteristic value
        create_task(bodySensorLocationCharacteristics->GetAt(0)->ReadValueAsync())
            .then([this] (GattReadResult^ readResult)
        {
            if (readResult->Status == GattCommunicationStatus::Success)
            {
                Array<unsigned char>^ bodySensorLocationData = 
                    ref new Array<unsigned char>(readResult->Value->Length);

                DataReader::FromBuffer(readResult->Value)->ReadBytes(bodySensorLocationData);

                String^ bodySensorLocation = HeartRateService::Instance->ProcessBodySensorLocationData(
                    bodySensorLocationData);

                if (bodySensorLocation->Length() > 0)
                {
                    OutputTextBlock->Text = "The Body Sensor Location of your device is : " + bodySensorLocation;
                }
                else
                {
                    OutputTextBlock->Text = "The Body Sensor Location is not recognized by this application";
                }
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
                MainPage::Current->NotifyUser("Error: " + e->Message, 
                    NotifyType::ErrorMessage);
            };
        });
    }
    else
    {
        MainPage::Current->NotifyUser("Your device does not support the Body Sensor Location characteristic.", 
            NotifyType::StatusMessage);
    }
}
