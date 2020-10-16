//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// S2_ReadCharacteristicValue.xaml.h
// Declaration of the S2_ReadCharacteristicValue class
//

#pragma once
#include "S2_ReadCharacteristicValue.g.h"

namespace SDKSample
{
    namespace BluetoothGattHeartRate
    {
        /// <summary>
        /// An empty page that can be used on its own or navigated to within a Frame.
        /// </summary>
        public ref class S2_ReadCharacteristicValue sealed
        {
        public:
            S2_ReadCharacteristicValue();

        private:
            void ReadValueButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
        };
    }
}
