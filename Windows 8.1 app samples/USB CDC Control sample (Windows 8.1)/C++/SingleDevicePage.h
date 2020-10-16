//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#pragma once

namespace SDKSample
{
    namespace UsbCdcControl
    {
        public ref class UsbSerialPortInfo sealed
        {
        public:
            UsbSerialPortInfo(UsbSerialPort^ port, Platform::String^ id, Platform::String^ name)
            {
                this->Port = port;
                this->DeviceId = id;
                this->Name = name;
            }

            property UsbSerialPort^ Port;

            property Platform::String^ DeviceId;

            property Platform::String^ Name;
        };

        /// <summary>
        /// This holds UsbSerialPort instance(s) between scenarios.
        /// </summary>
        public ref class UsbDeviceList sealed
        {
        internal:
            delegate Windows::Foundation::IAsyncAction^ DeviceRemovedHandler(Platform::Object^ sender, UsbDeviceInfo^ e);
            event Windows::Foundation::EventHandler<UsbDeviceInfo^>^ DeviceAdded;
            event DeviceRemovedHandler^ DeviceRemoved;

            static property UsbDeviceList^ Singleton
            {
                UsbDeviceList^ get()
                {
                    if (UsbDeviceList::singleton == nullptr)
                    {
                        UsbDeviceList::singleton = ref new UsbDeviceList();
                    }
                    return UsbDeviceList::singleton;
                }
            }

            void StartWatcher()
            {
                if (!DeviceList::Current->WatcherStarted)
                {
                    this->onDeviceAddedRegToken = DeviceList::Current->DeviceAdded::add(
                        ref new DeviceList::DeviceAddedHandler(this, &UsbDeviceList::OnDeviceAdded));

                    this->onDeviceRemovedRegToken = DeviceList::Current->DeviceRemoved::add(
                        ref new DeviceList::DeviceRemovedHandler(this, &UsbDeviceList::OnDeviceRemoved));

                    DeviceList::Current->StartWatcher();
                }
            }

            /// <summary>
            /// Call Dispose() for a UsbDevice.
            /// </summary>
            void DisposeDevice(Platform::String^ deviceId)
            {
                for (unsigned int i = 0; i < this->List->Size; i++)
                {
                    auto portInfo = this->List->GetAt(i);
                    if (portInfo->DeviceId == deviceId)
                    {
                        // Dispose the UsbDevice.
                        delete portInfo->Port->UsbDevice;
                        this->List->RemoveAt(i);
                        return;
                    }
                }
            }

            /// <summary>
            /// Call Dispose() for each UsbDevice, and call List.Clear().
            /// </summary>
            void DisposeAll()
            {
                for (unsigned int i = 0; i < this->List->Size; i++)
                {
                    auto portInfo = this->List->GetAt(i);
                    delete portInfo->Port->UsbDevice;
                }
                this->List->Clear();
            }

            property Platform::Collections::Vector<UsbSerialPortInfo^>^ List
            {
                Platform::Collections::Vector<UsbSerialPortInfo^>^ get() { return this->list; }
            }

        private:
            Platform::Collections::Vector<UsbSerialPortInfo^>^ list;
            static UsbDeviceList^ singleton;
            Windows::Foundation::EventRegistrationToken onDeviceAddedRegToken;
            Windows::Foundation::EventRegistrationToken onDeviceRemovedRegToken;

            UsbDeviceList()
            {
                this->list = ref new Platform::Collections::Vector<UsbSerialPortInfo^>();
            }

            ~UsbDeviceList()
            {
                DeviceList::Current->StopWatcher();
                DeviceList::Current->DeviceAdded::remove(this->onDeviceAddedRegToken);
                DeviceList::Current->DeviceRemoved::remove(this->onDeviceRemovedRegToken);
            }

            void OnDeviceAdded(Platform::Object^ sender, UsbDeviceInfo^ info)
            {
                DeviceAdded(sender, info);
            }

            void OnDeviceRemoved(Platform::Object^ sender, UsbDeviceInfo^ info)
            {
                auto asyncAction = DeviceRemoved(sender, info);
                if (!!asyncAction)
                {
                    concurrency::create_task(asyncAction).then([this, info]()
                    {
                        this->DisposeDevice(info->Id);
                    });
                }
                else
                {
                    this->DisposeDevice(info->Id);
                }
            }
        };

        /// <summary>
        /// Base page class, which uses just one device.
        /// </summary>
        public ref class SingleDevicePage : SDKSample::Common::LayoutAwarePage
        {
        internal:
            SingleDevicePage()
            {
            }

        protected:
            property UsbSerialPortInfo^ SerialPortInfo
            {
                UsbSerialPortInfo^ get()
                {
                    if (UsbDeviceList::Singleton->List->Size > 0)
                    {
                        return UsbDeviceList::Singleton->List->GetAt(0);
                    }
                    return nullptr;
                }
            }
        };
    }
}