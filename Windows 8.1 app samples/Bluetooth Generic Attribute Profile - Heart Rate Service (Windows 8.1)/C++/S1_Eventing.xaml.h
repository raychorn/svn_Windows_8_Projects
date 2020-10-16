//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

//
// S1_Eventing.xaml.h
// Declaration of the S1_Eventing class
//

#pragma once
#include "S1_Eventing.g.h"

namespace SDKSample
{
    namespace BluetoothGattHeartRate
    {
        /// <summary>
        /// An empty page that can be used on its own or navigated to within a Frame.
        /// </summary>
        public ref class S1_Eventing sealed
        {
        public:
            S1_Eventing();

        protected:
            virtual void LoadState(
                Platform::Object^ navigationParameter, 
                Windows::Foundation::Collections::IMap<Platform::String^, Platform::Object^>^ pageState) override;
            virtual void SaveState(
                Windows::Foundation::Collections::IMap<Platform::String^, Platform::Object^>^ pageState) override;

        private:
            void Instance_ValueChangeCompleted(HeartRateMeasurement^ heartRateMeasurement);
            void RefreshBluetoothDevices_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

            void OnSizeChanged(Platform::Object ^sender, Windows::UI::Xaml::SizeChangedEventArgs ^e);

            Windows::Foundation::EventRegistrationToken valueChangeCompletedRegistrationToken;
        };
    }
}
