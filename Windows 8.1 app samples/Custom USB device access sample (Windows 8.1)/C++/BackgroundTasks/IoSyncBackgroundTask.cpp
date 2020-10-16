//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#include "pch.h"
#include "IoSyncBackgroundTask.h"

using namespace Concurrency;
using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Devices::Background;
using namespace Windows::Devices::Usb;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace BackgroundTasks;

IoSyncBackgroundTask::IoSyncBackgroundTask()
{
}

void IoSyncBackgroundTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    auto deferral = Platform::Agile<BackgroundTaskDeferral^>(taskInstance->GetDeferral());
    backgroundTaskInstance = taskInstance;

    deviceSyncDetails = dynamic_cast<DeviceUseDetails^>(taskInstance->TriggerDetails);

    backgroundTaskInstance->Progress = 0;

    cancellationTokenSource = cancellation_token_source();

    taskInstance->Canceled += ref new BackgroundTaskCanceledEventHandler(this, &IoSyncBackgroundTask::OnCanceled);

    // After opening the device, sync with the device.
    // For simplicity, no error checking will be done after opening the device. Ideally, one should always
    // check if the device was successfully opened and respond accordingly. For an example on how to do this,
    // please see Scenario 1 of this sample.
    OpenDeviceAsync().then([this] ()
    {
        // The sample only demonstrates a bulk write for simplicity.
        // IO operations can be done after opening the device.
        // For more information on BackgroundTasks, please see the BackgroundTask sample on MSDN.
        return WriteToDeviceAsync();
    }, cancellationTokenSource.get_token()).then([this, deferral] (task<uint32> backgroundRunTask)
    {
        try
        {
            // Close the device because we are finished syncing and so that the app may reopen the device
            delete device;

            device = nullptr;

            auto bytesWritten = backgroundRunTask.get();

            ApplicationData::Current->LocalSettings->Values->Insert(LocalSettingKeys::SyncBackgroundTaskResult, bytesWritten.ToString());

            ApplicationData::Current->LocalSettings->Values->Insert(LocalSettingKeys::SyncBackgroundTaskStatus, BackgroundTaskInformation::TaskCompleted);
        }
        catch (const task_canceled& /* taskCanceled */)
        {
            ApplicationData::Current->LocalSettings->Values->Insert(LocalSettingKeys::SyncBackgroundTaskResult, "0");

            ApplicationData::Current->LocalSettings->Values->Insert(LocalSettingKeys::SyncBackgroundTaskStatus, BackgroundTaskInformation::TaskCanceled);
        }
        
        // Complete the background task (this raises the OnCompleted event on the corresponding BackgroundTaskRegistration)
        deferral->Complete();
    });
}

task<void> IoSyncBackgroundTask::OpenDeviceAsync()
{
    return create_task(UsbDevice::FromIdAsync(deviceSyncDetails->DeviceId), cancellationTokenSource.get_token()).then([this] (UsbDevice^ usbDevice)
    {
        device = usbDevice;

        backgroundTaskInstance->Progress = 10;
    });
}

/// <summary>
/// Cancels opening device and the IO operation, whichever is still running
/// </summary>
/// <param name="sender"></param>
/// <param name="reason"></param>
void IoSyncBackgroundTask::OnCanceled(IBackgroundTaskInstance^ /* sender */, BackgroundTaskCancellationReason /* reason */)
{
    cancellationTokenSource.cancel();

    backgroundTaskInstance->Progress = 0;
}

/// <summary>
/// Writes to device's first bulkOut endpoint of the default interface and updates progress per write.
/// When this method finishes, the progress will be 100.
/// </summary>
/// <returns>Total number of bytes written to the device</returns>
task<uint32> IoSyncBackgroundTask::WriteToDeviceAsync()
{
    return task<uint32>([this] ()
    {
        auto cancellationTokenForIo = cancellationTokenSource.get_token();

        uint32 totalBytesWritten = 0;

        if (!cancellationTokenForIo.is_canceled())
        {
            auto firstBulkOutEndpoint = device->DefaultInterface->BulkOutPipes->GetAt(0);

            // Evenly distributes the remaining progress (out of 100) among each write
            uint32 progressIncreasePerWrite = (100 - backgroundTaskInstance->Progress) / Sync::NumberOfTimesToWrite;

            auto dataWriter = ref new DataWriter(firstBulkOutEndpoint->OutputStream);

            // Create an array, all default initialized to 0, and write it to the buffer
            // The data inside the buffer will be garbage
            auto arrayBuffer = ref new Array<uint8>(Sync::BytesToWriteAtATime);

            for (uint32 timesWritten = 0; timesWritten < Sync::NumberOfTimesToWrite; timesWritten++)
            {
                if (!cancellationTokenForIo.is_canceled())
                {
                    dataWriter->WriteBytes(arrayBuffer);

                    // Wait for the store to complete
                    auto writeTask = create_task(dataWriter->StoreAsync(), cancellationTokenForIo);
                    writeTask.wait();

                    totalBytesWritten += writeTask.get();

                    backgroundTaskInstance->Progress += progressIncreasePerWrite;
                }
            }
        }

        return totalBytesWritten;
    });
}