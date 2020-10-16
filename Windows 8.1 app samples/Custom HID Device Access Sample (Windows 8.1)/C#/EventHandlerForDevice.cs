//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.ApplicationModel;
using Windows.Devices.Enumeration;
using Windows.Devices.HumanInterfaceDevice;
using Windows.Foundation;
using Windows.Storage;
using Windows.UI.Core;
using Windows.UI.Xaml;
using SDKTemplate;

namespace CustomHidDeviceAccess
{
    /// <summary>
    /// The purpose of this class is to demonstrate what to do to a HidDevice when a specific app event
    /// is raised (app suspension and resume) or when the device is disconnected. In addition to handling
    /// the HidDevice, the app's state should also be saved upon app suspension (will not be demonstrated here).
    /// 
    /// This class will also demonstrate how to handle device watcher events.
    /// 
    /// For simplicity, this class will only allow at most one device to be connected at any given time. In order
    /// to make this class support multiple devices, make this class a non-singleton and create multiple instances
    /// of this class; each instance should watch one connected device.
    /// </summary>
    class EventHandlerForDevice
    {
        /// <summary>
        /// Allows for singleton EventHandlerForDevice
        /// </summary>
        private static EventHandlerForDevice eventHandlerForDevice;

        private DeviceWatcher deviceWatcher;
        private String deviceSelector;

        private DeviceInformation deviceInformation;
        private HidDevice device;

        private SuspendingEventHandler appSuspendEventHandler;
        private EventHandler<Object> appResumeEventHandler;

        private TypedEventHandler<DeviceWatcher, DeviceInformation> deviceAddedEventHandler;
        private TypedEventHandler<DeviceWatcher, DeviceInformationUpdate> deviceRemovedEventHandler;

        private Boolean watcherSuspended;
        private Boolean watcherStarted;
        private Boolean isEnabledAutoReconnect;

        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        private MainPage rootPage = MainPage.Current;

        /// <summary>
        /// Enforces the singleton pattern so that there is only one object handling app events
        /// as it relates to the UsbDevice because this sample app only supports communicating with one device at a time. 
        ///
        /// An instance of EventHandlerForDevice is globally available because the device needs to persist across scenario pages.
        ///
        /// If there is no instance of EventHandlerForDevice created before this property is called,
        /// an EventHandlerForDevice will be created.
        /// </summary>
        public static EventHandlerForDevice Current
        {
            get
            {
                if (eventHandlerForDevice == null)
                {
                    CreateNewEventHandlerForDevice();
                }

                return eventHandlerForDevice;
            }
        }

        /// <summary>
        /// Creates a new instance of EventHandlerForDevice and uses it as the Current instance.
        /// </summary>
        public static void CreateNewEventHandlerForDevice()
        {
            eventHandlerForDevice = new EventHandlerForDevice();
        }

        public Boolean IsDeviceConnected
        {
            get
            {
                return (device != null);
            }
        }

        public HidDevice Device
        {
            get
            {
                return device;
            }
        }

        /// <summary>
        /// This DeviceInformation represents which device is connected or which device will be reconnected when
        /// the device is plugged in again (if IsEnabledAutoReconnect is true);.
        /// </summary>
        public DeviceInformation DeviceInformation
        {
            get
            {
                return deviceInformation;
            }
        }

        /// <summary>
        /// True if EventHandlerForDevice will attempt to reconnect to the device once it is plugged into the computer again
        /// </summary>
        public Boolean IsEnabledAutoReconnect
        {
            get
            {
                return isEnabledAutoReconnect;
            }
            set
            {
                isEnabledAutoReconnect = value;
            }
        }

        /// <summary>
        /// This method opens the device using the WinRT Hid API. After the device is opened, save the device
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
        public async Task<Boolean> OpenDeviceAsync(DeviceInformation deviceInfo, String deviceSelector)
        {
            deviceInformation = deviceInfo;
            this.deviceSelector = deviceSelector;

            // This sample uses FileAccessMode.ReadWrite to open the device because we do not want other apps opening our device and 
            // changing the state of our device. FileAccessMode.Read can be used instead. When FileAccessMode.Read is used, other apps will 
            // be able to send/get reports even if you have the device opened.
            return await HidDevice.FromIdAsync(deviceInformation.Id, FileAccessMode.ReadWrite).AsTask().ContinueWith<Boolean>((Task<HidDevice> deviceTask) =>
            {
                Boolean successfullyOpenedDevice = false;
                NotifyType notificationStatus;
                String notificationMessage = null;

                device = deviceTask.Result;

                // Device could have been blocked by user or the device has already been opened by another app.
                if (device != null)
                {
                    successfullyOpenedDevice = true;

                    notificationStatus = NotifyType.StatusMessage;
                    notificationMessage = "Device " + deviceInformation.Id + " opened";

                    // Create and register device watcher events for the device to be opened unless we're reopening the device
                    if (deviceWatcher == null)
                    {
                        deviceWatcher = DeviceInformation.CreateWatcher(deviceSelector);
                    }

                    if (!watcherStarted)
                    {
                        RegisterForAppEvents();
                        RegisterForDeviceWatcherEvents();

                        // Start the device watcher after we made sure that the device is opened.
                        StartDeviceWatcher();
                    }
                }
                else
                {
                    successfullyOpenedDevice = false;

                    notificationStatus = NotifyType.ErrorMessage;

                    var deviceAccessStatus = DeviceAccessInformation.CreateFromId(deviceInformation.Id).CurrentStatus;

                    if (deviceAccessStatus == DeviceAccessStatus.DeniedByUser)
                    {
                        notificationMessage = "Access to the device was blocked by the user : " + deviceInformation.Id;
                    }
                    else if (deviceAccessStatus == DeviceAccessStatus.DeniedBySystem)
                    {
                        // This status is most likely caused by app permissions (did not declare the device in the app's package.appxmanifest)
                        // This status does not cover the case where the device is already opened by another app.
                        notificationMessage = "Access to the device was blocked by the system : " + deviceInformation.Id;
                    }
                    else
                    {
                        // Most likely the device is opened by another app, but cannot be sure
                        notificationMessage = "Unknown error, possibly opened by another app : " + deviceInformation.Id;
                    }
                }

                var printMessageTask = rootPage.Dispatcher.RunAsync(
                    CoreDispatcherPriority.Normal,
                    new DispatchedHandler(() =>
                    {
                        MainPage.Current.NotifyUser(notificationMessage, notificationStatus);
                    }));

                return successfullyOpenedDevice;
            });
        }

        /// <summary>
        /// Closes the device and stops the device watcher.
        /// </summary>
        public void CloseDevice()
        {
            if (IsDeviceConnected)
            {
                CloseCurrentlyConnectedDevice();
            }

            if (deviceWatcher != null)
            {
                if (watcherStarted)
                {
                    StopDeviceWatcher();
                }

                deviceWatcher = null;
            }
        }

        private EventHandlerForDevice()
        {
            watcherStarted = false;
            watcherSuspended = false;
            isEnabledAutoReconnect = true;
        }

        /// <summary>
        /// This method demonstrates how to close the device properly using the WinRT Hid API.
        ///
        /// When the HidDevice is closing, it will cancel all IO operations that are still pending (not complete).
        /// The close will not wait for any IO completion callbacks to be called, so the close call may complete before any of
        /// the IO completion callbacks are called.
        /// The pending IO operations will still call their respective completion callbacks with either a task 
        /// cancelled error or the operation completed.
        /// </summary>
        private async void CloseCurrentlyConnectedDevice()
        {
            if (device != null)
            {
                // This closes the handle to the device
                device.Dispose();

                device = null;

                await rootPage.Dispatcher.RunAsync(
                    CoreDispatcherPriority.Normal,
                    new DispatchedHandler(() =>
                    {
                        MainPage.Current.NotifyUser(deviceInformation.Id + " is closed", NotifyType.StatusMessage);
                    }));
            }
        }

        /// <summary>
        /// Register for app suspension/resume events. See the comments
        /// for the event handlers for more information on what is being done to the device.
        ///
        /// We will also register for when the app exists so that we may close the device handle.
        /// </summary>
        private void RegisterForAppEvents()
        {
            appSuspendEventHandler = new SuspendingEventHandler(EventHandlerForDevice.Current.OnAppSuspension);
            appResumeEventHandler = new EventHandler<Object>(EventHandlerForDevice.Current.OnAppResume);

            // This event is raised when the app is exited and when the app is suspended
            App.Current.Suspending += appSuspendEventHandler;

            App.Current.Resuming += appResumeEventHandler;
        }

        /// <summary>
        /// Register for Added, Removed, and EnumerationCompleted events.
        /// </summary>
        private void RegisterForDeviceWatcherEvents()
        {
            deviceAddedEventHandler = new TypedEventHandler<DeviceWatcher, DeviceInformation>(this.OnDeviceAdded);

            deviceRemovedEventHandler = new TypedEventHandler<DeviceWatcher, DeviceInformationUpdate>(this.OnDeviceRemoved);

            deviceWatcher.Added += deviceAddedEventHandler;

            deviceWatcher.Removed += deviceRemovedEventHandler;
        }

        private void StartDeviceWatcher()
        {
            watcherStarted = true;

            if ((deviceWatcher.Status != DeviceWatcherStatus.Started)
                && (deviceWatcher.Status != DeviceWatcherStatus.EnumerationCompleted))
            {
                deviceWatcher.Start();
            }
        }

        private void StopDeviceWatcher()
        {
            if ((deviceWatcher.Status == DeviceWatcherStatus.Started)
                || (deviceWatcher.Status == DeviceWatcherStatus.EnumerationCompleted))
            {
                deviceWatcher.Stop();
            }

            watcherStarted = false;
        }

        /// <summary>
        /// If a HidDevice object has been instantiated (a handle to the device is opened), we must close it before the app 
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
        private void OnAppSuspension(Object sender, SuspendingEventArgs args)
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

            CloseCurrentlyConnectedDevice();
        }

        /// <summary>
        /// When resume into the application, we should reopen a handle to the Hid device again. This will automatically
        /// happen when we start the device watcher again; the device will be re-enumerated and we will attempt to reopen it
        /// if IsEnabledAutoReconnect property is enabled.
        /// 
        /// See OnAppSuspension for why we are starting the device watcher again
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="arg"></param>
        private void OnAppResume(Object sender, Object args)
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
        private void OnDeviceRemoved(DeviceWatcher sender, DeviceInformationUpdate deviceInformationUpdate)
        {
            if (deviceInformationUpdate.Id == deviceInformation.Id)
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
        private async void OnDeviceAdded(DeviceWatcher sender, DeviceInformation deviceInfo)
        {
            await rootPage.Dispatcher.RunAsync(
                CoreDispatcherPriority.Normal,
                new DispatchedHandler(async () =>
                {
                    if ((deviceInfo.Id == deviceInformation.Id) && !IsDeviceConnected && isEnabledAutoReconnect)
                    {
                        await OpenDeviceAsync(deviceInformation, deviceSelector);

                        // Any app specific device intialization should be done here because we don't know the state of the device when it is re-enumerated.
                    }
                }));
        }
    }
}
