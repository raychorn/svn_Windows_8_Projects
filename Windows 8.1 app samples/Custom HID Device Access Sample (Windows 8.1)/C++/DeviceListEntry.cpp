//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

#include "pch.h"
#include "DeviceListEntry.h"

using namespace Platform;

namespace SDKSample
{
    namespace CustomHidDeviceAccess
    {
        String^ DeviceListEntry::InstanceId::get(void)
        {
            return dynamic_cast<String^>(device->Properties->Lookup(const_cast<String^>(DeviceProperties::DeviceInstanceId)));
        }

        /// <summary>
        /// The class is mainly used as a DeviceInformation wrapper so that the UI can bind to a list of these.
        /// </summary>
        /// <param name="deviceInformation"></param>
        /// <param name="deviceSelector">The AQS used to find this device</param>
        DeviceListEntry::DeviceListEntry(Windows::Devices::Enumeration::DeviceInformation^ deviceInformation, String^ deviceSelector) :
            device(deviceInformation),
            deviceSelector(deviceSelector)
        {
        }
    }
}