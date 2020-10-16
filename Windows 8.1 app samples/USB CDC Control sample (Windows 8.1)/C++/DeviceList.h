//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#pragma once

namespace SDKSample {

    namespace UsbCdcControl {

        private ref class DeviceListEntry
        {
            Windows::Devices::Enumeration::DeviceInformation^ device;

        internal:

            property Platform::String^ InstanceId {
                Platform::String^ get(void) {
                    return safe_cast<Platform::String^>(this->device->Properties->Lookup("System.Devices.DeviceInstanceId"));
                }
            }

            property Platform::String^ Id {
                Platform::String^ get(void) {
                    return this->device->Id;
                }
            }

            property Platform::String^ Name {
                Platform::String^ get(void) {
                    return this->device->Name;
                }
            }

            property bool Matched;

            DeviceListEntry(Windows::Devices::Enumeration::DeviceInformation^ DeviceInterface) {
                this->device = DeviceInterface;
                Matched = true;
            }
        };

        public ref class UsbDeviceInfo sealed
        {
        internal:
            UsbDeviceInfo(Platform::String^ id, Platform::String^ name)
            {
                Id = id;
                Name = name;
            }

            UsbDeviceInfo(DeviceListEntry^ info)
            {
                Id = info->Id;
                Name = info->Name;
            }

        public:
            property Platform::String^ Name;
            property Platform::String^ Id;
        };

        public ref class DeviceList sealed
        {
        public:
            typedef Windows::Foundation::EventHandler<UsbDeviceInfo^> DeviceAddedHandler;
            typedef Windows::Foundation::EventHandler<UsbDeviceInfo^> DeviceRemovedHandler;

            static property DeviceList^ Current {DeviceList^ get(); };

            property bool WatcherStarted {
                bool get(void) {
                    return this->watcherStarted;
                }
                void set(bool value) {
                    this->watcherStarted = value;
                }
            };

            void StartWatcher();

            void StopWatcher();

            event DeviceAddedHandler^ DeviceAdded;
            event DeviceRemovedHandler^ DeviceRemoved;

        internal:
            property Windows::Foundation::Collections::IObservableVector<DeviceListEntry^>^ Devices {
                Windows::Foundation::Collections::IObservableVector<DeviceListEntry^>^ get(void){
                    return this->list;
                }
            };

        private:
            static DeviceList^ current;
            Windows::Devices::Enumeration::DeviceWatcher^ watcher;
            Platform::Collections::Vector<DeviceListEntry^>^ list;

            bool watcherSuspended;
            bool watcherStarted;

            DeviceList();

            DeviceListEntry^ FindDevice(Platform::String^ Id);

            void InitDeviceWatcher();

            void SuspendDeviceWatcher(Object^ Sender, Windows::ApplicationModel::SuspendingEventArgs^ Args);
            void ResumeDeviceWatcher(Object^ Sender, Object^ Args);

            void OnAdded(Windows::Devices::Enumeration::DeviceWatcher^ Sender, Windows::Devices::Enumeration::DeviceInformation^ DevInformation);
            void OnRemoved(Windows::Devices::Enumeration::DeviceWatcher^ Sender, Windows::Devices::Enumeration::DeviceInformationUpdate^ DevInformation);
            void OnEnumerationComplete(Windows::Devices::Enumeration::DeviceWatcher^ Sender, Object^ Args);
        };

    }
}