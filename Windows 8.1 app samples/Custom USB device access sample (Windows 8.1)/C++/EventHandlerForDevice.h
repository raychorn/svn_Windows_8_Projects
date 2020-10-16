//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#pragma once

#include "pch.h"

namespace SDKSample
{
    namespace CustomUsbDeviceAccess
    {
        /// <summary>
        /// The purpose of this class is to demonstrate what to do to a UsbDevice when a specific app event
        /// is raised (app suspension and resume) or when the device is disconnected. In addition to handling
        /// the UsbDevice, the app's state should also be saved upon app suspension (will not be demonstrated here).
        /// 
        /// This class will also demonstrate how to handle device watcher events.
        /// 
        /// For simplicity, this class will only allow at most one device to be connected at any given time. In order
        /// to make this class support multiple devices, make this class a non-singleton and create multiple instances
        /// of this class; each instance should watch one connected device.
        /// </summary>
        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class EventHandlerForDevice sealed
        {
        public:
            static property EventHandlerForDevice^ Current
            {
                EventHandlerForDevice^ get(void);
            }

            static void CreateNewEventHandlerForDevice(void);
            static void CreateNewEventHandlerForDeviceForBackgroundTasks(void);

            property Windows::UI::Xaml::SuspendingEventHandler^ OnAppSuspendCallback
            {
                Windows::UI::Xaml::SuspendingEventHandler^ get(void)
                {
                    return appSuspendCallback;
                }

                void set(Windows::UI::Xaml::SuspendingEventHandler^ newSuspensionHandler)
                {
                    appSuspendCallback = newSuspensionHandler;
                }
            }

            property bool IsDeviceConnected
            {
                bool get(void)
                {
                    return device != nullptr;
                }
            }

            property Windows::Devices::Usb::UsbDevice^ Device
            {
                Windows::Devices::Usb::UsbDevice^ get(void)
                {
                    return device;
                }
            }

            /// <summary>
            /// This DeviceInformation represents which device is connected or which device will be reconnected when
            /// the device is plugged in again (if IsEnabledAutoReconnect is true);.
            /// </summary>
            property Windows::Devices::Enumeration::DeviceInformation^ DeviceInformation
            {
                Windows::Devices::Enumeration::DeviceInformation^ get(void)
                {
                    return deviceInformation;
                }
            }

            /// <summary>
            /// DeviceSelector AQS used to find this device
            /// </summary>
            property Platform::String^ DeviceSelector
            {
                Platform::String^ get(void)
                {
                    return deviceSelector;
                }
            }

            /// <summary>
            /// True if EventHandlerForDevice will attempt to reconnect to the device once it is plugged into the computer again
            /// </summary>
            property bool IsEnabledAutoReconnect
            {
                bool get(void)
                {
                    return isEnabledAutoReconnect;
                }
                void set(bool value)
                {
                    isEnabledAutoReconnect = value;
                }
            }

            Windows::Foundation::IAsyncOperation<bool>^ OpenDeviceAsync(Windows::Devices::Enumeration::DeviceInformation^ deviceInfo, Platform::String^ deviceSelector);

            void CloseDevice(void);
        private:
            /// <summary>
            /// Allows for singleton EventHandlerForDevice
            /// </summary>
            static EventHandlerForDevice^ eventHandlerForDevice;

            Windows::Devices::Enumeration::DeviceWatcher^ deviceWatcher;
            Platform::String^ deviceSelector;

            Windows::Devices::Enumeration::DeviceInformation^ deviceInformation;
            Windows::Devices::Usb::UsbDevice^ device;

            Windows::Foundation::EventRegistrationToken appSuspendEventToken;
            Windows::Foundation::EventRegistrationToken appResumeEventToken;

            Windows::Foundation::EventRegistrationToken deviceAddedEventToken;
            Windows::Foundation::EventRegistrationToken deviceRemovedEventToken;
            
            Windows::UI::Xaml::SuspendingEventHandler^ appSuspendCallback;

            bool watcherSuspended;
            bool watcherStarted;
            bool isEnabledAutoReconnect;
            bool isBackgroundTask;

            EventHandlerForDevice(bool isBackgroundTask);

            void CloseCurrentlyConnectedDevice(void);

            void RegisterForAppEvents(void);
            void UnregisterFromAppEvents(void);
            
            void RegisterForDeviceWatcherEvents(void);

            void StartDeviceWatcher(void);
            void StopDeviceWatcher(void);

            void OnAppSuspension(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
            void OnAppResume(Platform::Object^ sender, Platform::Object^ args);

            void OnDeviceRemoved(
                Windows::Devices::Enumeration::DeviceWatcher^ sender, 
                Windows::Devices::Enumeration::DeviceInformationUpdate^ deviceInformationUpdate);

            void OnDeviceAdded(
                Windows::Devices::Enumeration::DeviceWatcher^ sender, 
                Windows::Devices::Enumeration::DeviceInformation^ deviceInfo);
        };
    }
}
