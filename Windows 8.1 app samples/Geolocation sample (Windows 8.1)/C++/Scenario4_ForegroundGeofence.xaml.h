//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

//
// Scenario4_ForegroundGeofence.xaml.h
// Declaration of the Scenario4 class
//

#pragma once

#include "pch.h"
#include "Scenario4_ForegroundGeofence.g.h"
#include "MainPage.xaml.h"

namespace SDKSample
{
    namespace GeolocationCPP
    {
        // Since data binding uses Platform::Collections::Vector<Object^>^
        // the GeofenceItem must be implemented as a WinRT object.
        // This means that native data types cannot be used in
        // the interface.
        // Note that this is different than the 
        // implementation in C# where native types
        // are allowed because the collection uses
        // System.Collections.ObjectModel.ObservableCollection<GeofenceItem>
        // but System isn't available in C++ Windows Store Apps.
        // Also note the metadata that allows this class
        // to be used in data binding.
        // Also note that since System isn't available
        // GeofenceItem does not implement IEquatable to allow
        // removal of objects in the collection
        [Windows::UI::Xaml::Data::Bindable]
        public ref class GeofenceItem sealed
        {
        private:
            Windows::Devices::Geolocation::Geofencing::Geofence^ geofence;

        public:
            GeofenceItem(Windows::Devices::Geolocation::Geofencing::Geofence^ geofence)
            {
                this->geofence = geofence;
            }

            property Windows::Devices::Geolocation::Geofencing::Geofence^ Geofence
            {
                Windows::Devices::Geolocation::Geofencing::Geofence^ get();
            }

            property Platform::String^ Id
            {
                Platform::String^ get();
            }

            property double Latitude
            {
                double get();
            }

            property double Longitude
            {
                double get();
            }

            property double Radius
            {
                double get();
            }

            property long long Dwell
            {
                long long get();
            }

            property bool SingleUse
            {
                bool get();
            }

            property unsigned int MonitoredStates
            {
                unsigned int get();
            }

            property long long Duration
            {
                long long get();
            }

            property long long Start
            {
                long long get();
            }
        };

        /// <summary>
        /// An empty page that can be used on its own or navigated to within a Frame.
        /// </summary>
        [Windows::Foundation::Metadata::WebHostHidden]
        public ref class Scenario4 sealed
        {
        public:
            Scenario4();

        protected:
            virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
            virtual void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

        private:
            void RefreshControlsFromGeofenceItem(GeofenceItem^ item);
            bool TextChangedHandlerDouble(bool nullAllowed, Platform::String^ name, Windows::UI::Xaml::Controls::TextBox^ e);
            bool TextChangedHandlerInt(bool nullAllowed, Platform::String^ name, Windows::UI::Xaml::Controls::TextBox^ e);
            bool SettingsAvailable();
            void AddGeofenceToRegisteredGeofenceListView(Windows::Devices::Geolocation::Geofencing::Geofence^ geoFence);
            void GetGeopositionAsync();
            Windows::Devices::Geolocation::Geofencing::Geofence^ GenerateGeofence();
            void FillRegisteredGeofenceListViewWithExistingGeofences();
            Platform::String^ GetTimeStampedMessage(Platform::String^ EventCalled);

            void OnGeofenceStateChangedHandler(Windows::Devices::Geolocation::Geofencing::GeofenceMonitor^ sender, Platform::Object^ args);
            void OnStatusChangedHandler(Windows::Devices::Geolocation::Geofencing::GeofenceMonitor^ sender, Platform::Object^ args);
            void OnAccessChangedHandler(Windows::Devices::Enumeration::DeviceAccessInformation^ sender, Windows::Devices::Enumeration::DeviceAccessChangedEventArgs^ args);
            void GeofenceNameTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceLatitudeTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceLongitudeTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceRadiusTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceDurationSecondTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceDurationMinuteTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceDurationHourTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceDurationDayTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceDwellTimeSecondTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceDwellTimeMinuteTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceDwellTimeHourTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceDwellTimeDayTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceStartTimeSecondTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceStartTimeMinuteTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceStartTimeHourTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceStartTimeDayTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceStartTimeMonthTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceStartTimeYearTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
            void GeofenceRegisteredListViewSelectionChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e);
            void Remove(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
            void CreateGeofence(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

            MainPage^ rootPage;
            bool latitudeSet;
            bool longitudeSet;
            bool radiusSet;
            bool permissionsChecked;
            bool inGetPositionAsync;
            bool geofenceStateChangedRegistered;
            Windows::Devices::Geolocation::Geolocator^ geolocator;
            Windows::Devices::Enumeration::DeviceAccessInformation^ accessInfo;
            Windows::Foundation::EventRegistrationToken accessToken;
            Windows::Foundation::EventRegistrationToken geofencestatechangeToken;
            Windows::Foundation::EventRegistrationToken geofencestatuschangeToken;
            concurrency::cancellation_token_source geopositionTaskTokenSource;
            Platform::Collections::Vector<Object^>^ geofenceCollection;
            Platform::Collections::Vector<Object^>^ geofenceEvents;
        };
    }
}
