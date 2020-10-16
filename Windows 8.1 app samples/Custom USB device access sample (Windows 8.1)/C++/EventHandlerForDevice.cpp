//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#include "pch.h"
#include "EventHandlerForDevice.h"
#include "MainPage.xaml.h"
#include "App.xaml.h"

using namespace Platform;
using namespace Concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::Devices;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Usb;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace SDKSample::Common;
using namespace SDKSample::CustomUsbDeviceAccess;

EventHandlerForDevice^ EventHandlerForDevice::eventHandlerForDevice = nullptr;

/// <summary>
/// Enforces the singleton pattern so that there is only one object handling app events
/// as it relates to the UsbDevice because this sample app only supports communicating with one device at a time. 
///
/// An instance of EventHandlerForDevice is globally available because the device needs to persist across scenario pages.
///
/// If there is no instance of EventHandlerForDevice created before this property is called,
/// an EventHandlerForDevice will be created; the EventHandlerForDevice created this way
/// is not meant for BackgroundTasks.
/// </summary>
EventHandlerForDevice^ EventHandlerForDevice::Current::get(void)
{
    if (eventHandlerForDevice == nullptr)
    {
        CreateNewEventHandlerForDevice();
    }
    
    return eventHandlerForDevice;
}



/// <summary>
/// Creates a new instance of EventHandlerForDevice, enables auto reconnect, and uses it as the Current instance.
/// </summary>
void EventHandlerForDevice::CreateNewEventHandlerForDevice(void)
{
    eventHandlerForDevice = ref new EventHandlerForDevice(false);
}

/// <summary>
/// Creates a new instance of EventHandlerForDevice, disables auto reconnect, and uses it as the Current instance.
/// </summary>
void EventHandlerForDevice::CreateNewEventHandlerForDeviceForBackgroundTasks(void)
{
    eventHandlerForDevice = ref new EventHandlerForDevice(true);
}

/// <summary>
/// This method opens the device using the WinRT Usb API. After the device is opened, we will save the device
/// so that it can be used across scenarios.
///
/// It is important that the FromIdAsync call is made on the UI thread because the consent prompt can only be displayed
/// on the UI thread.
/// 
/// This method is used to reopen the device after the device reconnects to the computer and when the app resumes.
/// </summary>
/// <param name="deviceInfo">Device information of the device to be opened</param>
/// <param name="deviceSelector">The AQS used to find this device</param>
/// <returns>True if the device was successfully opened, false if the device could not be opened for well known reasons.
/// An exception may be thrown if the device could not be opened for extraordinary reasons.</returns>
IAsyncOperation<bool>^ EventHandlerForDevice::OpenDeviceAsync(Enumeration::DeviceInformation^ deviceInfo, String^ deviceSelector)
{
    deviceInformation = deviceInfo;
    this->deviceSelector = deviceSelector;

    return create_async([this] () 
    {
        return create_task(UsbDevice::FromIdAsync(deviceInformation->Id))
            .then([this](task<UsbDevice^> deviceTask) 
            {
                bool successfullyOpenedDevice = false;
                NotifyType notificationStatus;
                String^ notificationMessage = nullptr;

                // This may throw an exception or return null if we could not open the device
                device = deviceTask.get();

                // Device could have been blocked by user or the device has already been opened by another app.
                if (device != nullptr)
                {
                    successfullyOpenedDevice = true;

                    notificationStatus = NotifyType::StatusMessage;
                    notificationMessage = "Device " + deviceInformation->Id + " opened";

                    // Create and register device watcher events for the device to be opened unless we're reopening the device
                    if (deviceWatcher == nullptr)
                    {
                        deviceWatcher = Enumeration::DeviceInformation::CreateWatcher(this->deviceSelector);
                    }

                    if (!watcherStarted)
                    {
                        // Background tasks are not part of the app, so app events will not have an affect on the device
                        if (!isBackgroundTask)
                        {
                            RegisterForAppEvents();
                        }

                        RegisterForDeviceWatcherEvents();

                        // Start the device watcher after we made sure that the device is opened.
                        StartDeviceWatcher();
                    }
                }
                else
                {
                    successfullyOpenedDevice = false;

                    notificationStatus = NotifyType::ErrorMessage;

                    auto deviceAccessStatus = DeviceAccessInformation::CreateFromId(deviceInformation->Id)->CurrentStatus;

                    if (deviceAccessStatus == DeviceAccessStatus::DeniedByUser)
                    {
                        notificationMessage = "Access to the device was blocked by the user : " + deviceInformation->Id;
                    }
                    else if (deviceAccessStatus == DeviceAccessStatus::DeniedBySystem)
                    {
                        // This status is most likely caused by app permissions (did not declare the device in the app's package.appxmanifest)
                        // This status does not cover the case where the device is already opened by another app.
                        notificationMessage = "Access to the device was blocked by the system : " + deviceInformation->Id;
                    }
                    else
                    {
                        // Most likely the device is opened by another app, but cannot be sure
                        notificationMessage = "Unknown error, possibly opened by another app : " + deviceInformation->Id;
                    }
                }

                MainPage::Current->NotifyUser(notificationMessage, notificationStatus);

                return successfullyOpenedDevice;
            });
    });
}

/// <summary>
/// Closes the device and stops the device watcher.
/// </summary>
void EventHandlerForDevice::CloseDevice(void)
{
    if (IsDeviceConnected)
    {
        CloseCurrentlyConnectedDevice();
    }

    if (deviceWatcher != nullptr)
    {
        if (watcherStarted)
        {
            StopDeviceWatcher();
        }

        deviceWatcher = nullptr;
    }

    UnregisterFromAppEvents();

    appSuspendCallback = nullptr;
}

/// <summary>
/// If this event handler will be running in a background task, app events will not be registered for because they are of
/// no use to the background task.
/// </summary>
/// <param name="isBackgroundTask">Whether or not the event handler will be running as a background task</param>
EventHandlerForDevice::EventHandlerForDevice(bool isBackgroundTask) :
    watcherStarted(false),
    watcherSuspended(false),
    isEnabledAutoReconnect(true),
    isBackgroundTask(isBackgroundTask)
{
}

/// <summary>
/// This method demonstrates how to close the device properly using the WinRT Usb API.
///
/// When the UsbDevice is closing, it will cancel all IO operations that are still pending (not complete).
/// The close will not wait for any IO completion callbacks to be called, so the close call may complete before any of
/// the IO completion callbacks are called.
/// The pending IO operations will still call their respective completion callbacks with either a task 
/// cancelled error or the operation completed.
/// </summary>
void EventHandlerForDevice::CloseCurrentlyConnectedDevice(void)
{
    if (device != nullptr)
    {
        // This closes the handle to the device
        delete device;

        device = nullptr;

        MainPage::Current->Dispatcher->RunAsync(
            CoreDispatcherPriority::Normal,
            ref new DispatchedHandler([this]()
        {
            MainPage::Current->NotifyUser(deviceInformation->Id + " is closed", NotifyType::StatusMessage);
        }));
    }
}

/// <summary>
/// Register for app suspension/resume events. See the comments
/// for the event handlers for more information on what is being done to the device.
///
/// We will also register for when the app exists so that we may close the device handle.
/// </summary>
void EventHandlerForDevice::RegisterForAppEvents()
{
    // This event is raised when the app is exited and when the app is suspended
    appSuspendEventToken = App::Current->Suspending += ref new SuspendingEventHandler(this, &EventHandlerForDevice::OnAppSuspension);

    appResumeEventToken = App::Current->Resuming += ref new EventHandler<Object^>(this, &EventHandlerForDevice::OnAppResume);
}

void EventHandlerForDevice::UnregisterFromAppEvents()
{
    App::Current->Suspending -= appSuspendEventToken;
    
    App::Current->Resuming -= appResumeEventToken;
}

/// <summary>
/// Register for Added, Removed, and EnumerationCompleted events.
/// </summary>
void EventHandlerForDevice::RegisterForDeviceWatcherEvents()
{
    deviceAddedEventToken = deviceWatcher->Added += ref new TypedEventHandler<DeviceWatcher^, Enumeration::DeviceInformation^>(
        this, &EventHandlerForDevice::OnDeviceAdded);

    deviceRemovedEventToken = deviceWatcher->Removed += ref new TypedEventHandler<DeviceWatcher^, DeviceInformationUpdate^>(
        this, &EventHandlerForDevice::OnDeviceRemoved);
}

void EventHandlerForDevice::StartDeviceWatcher(void)
{
    watcherStarted = true;

    if ((deviceWatcher->Status != DeviceWatcherStatus::Started)
        && (deviceWatcher->Status != DeviceWatcherStatus::EnumerationCompleted))
    {
        deviceWatcher->Start();
    }
}

void EventHandlerForDevice::StopDeviceWatcher(void)
{
    if ((deviceWatcher->Status == DeviceWatcherStatus::Started)
        || (deviceWatcher->Status == DeviceWatcherStatus::EnumerationCompleted))
    {
        deviceWatcher->Stop();
    }

    watcherStarted = false;
}

/// <summary>
/// If a UsbDevice object has been instantiated (a handle to the device is opened), we must close it before the app 
/// goes into suspension because the API automatically closes it for us if we don't. When resuming, the API will
/// not reopen the device automatically, so we need to explicitly open the device in the app (Scenario1_DeviceConnect).
///
/// Since we have to reopen the device ourselves when the app resumes, it is good practice to explicitly call the close
/// in the app as well (For every open there is a close).
/// 
/// We must stop the DeviceWatcher because it will continue to raise events even if
/// the app is in suspension, which is not desired (drains battery). We resume the device watcher once the app resumes again.
/// </summary>
/// <param name="sender"></param>
/// <param name="eventArgs"></param>
void EventHandlerForDevice::OnAppSuspension(Object^ sender , SuspendingEventArgs^ eventArgs )
{
    if (watcherStarted)
    {
        watcherSuspended = true;
        StopDeviceWatcher();
    }
    else
    {
        watcherSuspended = false;
    }

    // Forward suspend event to registered callback function
    if (appSuspendCallback != nullptr)
    {
        appSuspendCallback(sender, eventArgs);
    }

    CloseCurrentlyConnectedDevice();
}

/// <summary>
/// When resume into the application, we should reopen a handle to the Usb device again. This will automatically
/// happen when we start the device watcher again; the device will be re-enumerated and we will attempt to reopen it
/// if IsEnabledAutoReconnect property is enabled.
/// 
/// See OnAppSuspension for why we are starting the device watcher again
/// </summary>
/// <param name="sender"></param>
/// <param name="args"></param>
void EventHandlerForDevice::OnAppResume(Object^ /* sender */, Object^ /* args */) 
{
    if (watcherSuspended)
    {
        watcherSuspended = false;
        StartDeviceWatcher();
    }
}

/// <summary>
/// Close the device that is opened so that all pending operations are canceled properly.
/// </summary>
/// <param name="sender"></param>
/// <param name="deviceInformationUpdate"></param>
void EventHandlerForDevice::OnDeviceRemoved(DeviceWatcher^ /* sender */, DeviceInformationUpdate^ deviceInformationUpdate)
{
    if ((deviceInformationUpdate->Id == deviceInformation->Id) && IsDeviceConnected)
    {
        // The main reasons to close the device explicitly is to clean up resources, to properly handle errors,
        // and stop talking to the disconnected device.
        CloseCurrentlyConnectedDevice();
    }
}

/// <summary>
/// Open the device that the user wanted to open if it hasn't been opened yet and auto reconnect is enabled.
/// </summary>
/// <param name="sender"></param>
/// <param name="deviceInfo"></param>
void EventHandlerForDevice::OnDeviceAdded(DeviceWatcher^ /* sender */, Enumeration::DeviceInformation^ deviceInfo)
{
    if ((deviceInfo->Id == deviceInformation->Id) && !IsDeviceConnected && isEnabledAutoReconnect)
    {
        MainPage::Current->Dispatcher->RunAsync(
            CoreDispatcherPriority::Normal,
            ref new DispatchedHandler([this]()
        {
            OpenDeviceAsync(deviceInformation, deviceSelector);

            // Any app specific device intialization should be done here because we don't know the state of the device when it is re-enumerated.
        }));
    }
}
