#include "pch.h"
#include "HeartRateService.h"

#include "MainPage.xaml.h"

using namespace concurrency;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;
using namespace Windows::UI::Xaml;

using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

using namespace SDKSample::BluetoothGattHeartRate;

HeartRateService^ HeartRateService::instance = nullptr;

HeartRateService::HeartRateService()
{
    datapoints = ref new Vector<HeartRateMeasurement^>();
    App::Current->Suspending += ref new SuspendingEventHandler(this, &HeartRateService::App_Suspending);
    App::Current->Resuming += ref new EventHandler<Platform::Object^>(this, &HeartRateService::App_Resuming);
}


HeartRateService::~HeartRateService()
{
}

void HeartRateService::App_Suspending(Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e)
{
    isServiceInitialized = false;

    // Obtain RAII lock from dataLock
    reader_writer_lock::scoped_lock lock(dataLock);

    // This is an appropriate place to save to persistent storage any datapoint the application cares about.
    // For the purpose of this sample we just discard any values.
    datapoints->Clear();

    // Allow the GattDeviceService to get cleaned up by the Windows Runtime.
    // The Windows runtime will clean up resources used by the GattDeviceService object when the application gets 
    // suspended. The GattDeviceService object will be invalid once the app resumes, which is why it must be marked as
    // invalid, and reinitalized when the application resumes.
    delete service;
    service = nullptr;
}

void HeartRateService::App_Resuming(Object^ sender, Object^ e)
{
    // Since the Windows Runtime will close resources to the device when the app is suspended,
    // the device needs to be reinitialized when the app is resumed.
    InitializeHeartRateServices();
}

void HeartRateService::InitializeHeartRateServices()
{
    create_task(DeviceInformation::FindAllAsync(
        GattDeviceService::GetDeviceSelectorFromUuid(GattServiceUuids::HeartRate)))
        .then([this] (Windows::Devices::Enumeration::DeviceInformationCollection^ heartRateServices)
    {
        if (heartRateServices->Size > 0)
        {
            return create_task(GattDeviceService::FromIdAsync(heartRateServices->GetAt(0)->Id))
                .then([this] (GattDeviceService^ firstHeartRateService)
            {
                if (firstHeartRateService != nullptr)
                {
                    auto heartRateMeasurementCharacteristics = 
                        firstHeartRateService->GetCharacteristics(GattCharacteristicUuids::HeartRateMeasurement);
                    if (heartRateMeasurementCharacteristics->Size > 0)
                    {
                        GattCharacteristic^ heartRateMeasurementCharacteristic = 
                            heartRateMeasurementCharacteristics->GetAt(0);

                        this->service = firstHeartRateService;

                        heartRateMeasurementCharacteristic->ValueChanged += 
                            ref new TypedEventHandler<GattCharacteristic^, GattValueChangedEventArgs^>(
                                this, 
                                &HeartRateService::HeartRateMeasurementCharacteristic_ValueChanged);

                        return create_task(
                            heartRateMeasurementCharacteristic->WriteClientCharacteristicConfigurationDescriptorAsync(
                            GattClientCharacteristicConfigurationDescriptorValue::Notify))
                            .then([this] (GattCommunicationStatus result) 
                        {
                            if (result == GattCommunicationStatus::Success)
                            {
                                isServiceInitialized = true;
                            }
                            else
                            {
                                SDKSample::MainPage::Current->NotifyUser("Your device is unreachable, most likely " +
                                    "the device is out of range, or is running low on battery, please make sure " +
                                    "your device is working and try again.",
                                    NotifyType::StatusMessage);
                            }
                        });
                    }
                    else
                    {
                        MainPage::Current->NotifyUser(L"ERROR: Your Heart Rate device should have at least one " +
                            "Heart RateMeasurement characteristic, but none were found!",
                            NotifyType::ErrorMessage);
                    }
                }
                else
                {
                    MainPage::Current->NotifyUser("Access to the device is denied, because the application was not " +
                        "granted access, or the device is currently in use by another application.",
                        NotifyType::StatusMessage);
                }
                // The lambda must return a continuation, but the prerequisites to call
                // WriteClientCharacteristicConfigurationDescriptorAsync were not met, and its continuation could not
                // be returned, so a placeholder continuation must be returned instead.
                return create_task([] {});
            });
        }
        else
        {
            MainPage::Current->NotifyUser(L"Could not find any Heart Rate devices. Please make sure your " +
                "device is paired and powered on!",
                NotifyType::StatusMessage);
            
            // The lambda must return a continuation, but the prerequisites to call FromIdAsync were not met, and its
            // continuation could not be returned, so a placeholder continuation must be returned instead
            return create_task([] {});
        }
    })
        .then([] (task<void> finalTask)
    {
        try 
        {
            // Capture any errors and exceptions that occured
            finalTask.get();
        }
        catch (COMException^ e)
        {
            MainPage::Current->NotifyUser("Error: " + e->HResult.ToString() + " - " + e->Message, 
                NotifyType::ErrorMessage);
        }
    });
}

/// <summary>
/// Process the raw data read from the device into an application usable string, according to the Bluetooth
/// Heart Rate Profile.
/// </summary>
/// <param name="bodySensorLocationData">Raw data read from the heart rate monitor.</param>
/// <returns>The textual representation of the Body Sensor Location.</returns>
String^ HeartRateService::ProcessBodySensorLocationData(const Array<unsigned char>^ bodySensorLocationData)
{
    // The Bluetooth Heart Rate Profile specifies that the Body Sensor Location characteristic value has
    // a single byte of data
    unsigned char bodySensorLocationValue = bodySensorLocationData[0];
    String^ retval = "";
    switch (bodySensorLocationValue)
    {
    case 0:
        retval += "Other";
        break;
    case 1:
        retval += "Chest";
        break;
    case 2:
        retval += "Wrist";
        break;
    case 3:
        retval += "Finger";
        break;
    case 4:
        retval += "Hand";
        break;
    case 5:
        retval += "Ear Lobe";
        break;
    case 6:
        retval += "Foot";
        break;
    default:
        // By default we simply return an empty string
        break;
    }
    return retval;
}

/// <summary>
/// Process the raw data received from the device into application usable data, according to the Bluetooth Heart Rate Profile.
/// </summary>
/// <param name="heartRateMeasurementData">Raw data received from the heart rate monitor.</param>
/// <returns>The heart rate measurement value.</returns>
HeartRateMeasurement^ HeartRateService::ProcessHeartRateMeasurementData(Array<unsigned char>^ heartRateMeasurementData)
{
    // Heart Rate Profile defined flags
    const unsigned char HEART_RATE_VALUE_FORMAT = 0x01;
    const unsigned char ENERGY_EXPENDED_STATUS = 0x08;

    try 
    {
        unsigned char currentOffset = 0;
        unsigned char flags = heartRateMeasurementData[currentOffset];
        bool isHeartRateValueSizeLong = ((flags & HEART_RATE_VALUE_FORMAT) != 0);
        bool hasEnergyExpended = ((flags & ENERGY_EXPENDED_STATUS) != 0);

        currentOffset++;

        uint16 heartRateMeasurementValue = 0;

        if (isHeartRateValueSizeLong)
        {
            heartRateMeasurementValue = (uint16)((heartRateMeasurementData[currentOffset + 1] << 8) +
                heartRateMeasurementData[currentOffset]);
            currentOffset += 2;
        }
        else
        {
            heartRateMeasurementValue = heartRateMeasurementData[currentOffset];
            currentOffset++;
        }

        uint16 expendedEnergyValue = 0;

        if (hasEnergyExpended)
        {
            expendedEnergyValue = (uint16)((heartRateMeasurementData[currentOffset + 1] << 8) +
                heartRateMeasurementData[currentOffset]);
            currentOffset += 2;
        }
    
        // The Bluetooth Heart Rate profile can also contain sensor contact status information, and R-Wave interval measurements,
        // which can also be processed here. For the purpose of this sample, we don't need to interpret that data.

        auto retval = ref new HeartRateMeasurement();
        retval->HeartRateValue = heartRateMeasurementValue;
        retval->ExpendedEnergy = expendedEnergyValue;

        return retval;
    }
    catch (OutOfBoundsException^ e)
    {
        MainPage::Current->NotifyUser("Received malformed data from the device, which cannot be interpreted",
            NotifyType::ErrorMessage);

        return nullptr;
    }
}

/// <summary>
/// Invoked when Windows receives data from your Bluetooth device.
/// </summary>
/// <param name="sender">The GattCharacteristic object whose value is received.</param>
/// <param name="args">The new characteristic value sent by the device.</param>
void HeartRateService::HeartRateMeasurementCharacteristic_ValueChanged(
    GattCharacteristic^ sender, 
    GattValueChangedEventArgs^ args)
{
    auto heartRateMeasurementData = ref new Array<unsigned char>(args->CharacteristicValue->Length);

    DataReader::FromBuffer(args->CharacteristicValue)->ReadBytes(heartRateMeasurementData);

    auto heartRateValue = ProcessHeartRateMeasurementData(heartRateMeasurementData);

    // if correct data was received from the device, update the value
    if (heartRateValue != nullptr)
    {
        heartRateValue->Timestamp = args->Timestamp;

        {
            // Obtain RAII lock from dataLock
            reader_writer_lock::scoped_lock lock(dataLock);

            datapoints->Append(heartRateValue);
        }

        ValueChangeCompleted(heartRateValue);
    }
}
