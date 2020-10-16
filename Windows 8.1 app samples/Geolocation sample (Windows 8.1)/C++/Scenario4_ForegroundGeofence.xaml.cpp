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
// Scenario4_ForegroundGeofence.xaml.cpp
// Implementation of the Scenario4 class
//

#include "pch.h"
#include "Scenario4_ForegroundGeofence.xaml.h"

using namespace SDKSample;
using namespace SDKSample::GeolocationCPP;

using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Geolocation;
using namespace Windows::Devices::Geolocation::Geofencing;
using namespace Windows::Globalization::DateTimeFormatting;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Platform;
using namespace concurrency;

const unsigned int secondsPerMinute = 60;
const unsigned int secondsPerHour = 60 * secondsPerMinute;
const unsigned int secondsPerDay = 24 * secondsPerHour;
const unsigned int nanosecondsPerSecond = 10000000;

const unsigned int maxDays = 999;
const unsigned int maxHours = 99;
const unsigned int maxMinutes = 99;
const unsigned int maxSeconds = 99;

Scenario4::Scenario4() : rootPage(MainPage::Current), latitudeSet(false), longitudeSet(false), radiusSet(false), permissionsChecked(false), inGetPositionAsync(false), geofenceStateChangedRegistered(false)
{
    InitializeComponent();

    try
    {
        geofenceCollection = ref new Platform::Collections::Vector<Object^>();
        geofenceEvents = ref new Platform::Collections::Vector<Object^>();

        // Get a geolocator object
        geolocator = ref new Geolocator();

        // using data binding to the root page collection of GeofenceItems
        GeofenceRegisteredListView->DataContext = geofenceCollection;

        // using data binding to the root page collection of GeofenceItems associated with events
        GeofenceEventsListView->DataContext = geofenceEvents;

        FillRegisteredGeofenceListViewWithExistingGeofences();

        accessInfo = DeviceAccessInformation::CreateFromDeviceClass(DeviceClass::Location);
        accessToken = accessInfo->AccessChanged += ref new TypedEventHandler<DeviceAccessInformation^, DeviceAccessChangedEventArgs^>(this, &SDKSample::GeolocationCPP::Scenario4::OnAccessChangedHandler);

        // register for state change events
        geofencestatechangeToken = GeofenceMonitor::Current->GeofenceStateChanged += ref new TypedEventHandler<GeofenceMonitor^, Platform::Object^>(this, &SDKSample::GeolocationCPP::Scenario4::OnGeofenceStateChangedHandler);
        geofencestatuschangeToken = GeofenceMonitor::Current->StatusChanged += ref new TypedEventHandler<GeofenceMonitor^, Platform::Object^>(this, &SDKSample::GeolocationCPP::Scenario4::OnStatusChangedHandler);

        geofenceStateChangedRegistered = true;
    }
    catch (Platform::AccessDeniedException^)
    {
        if (DeviceAccessStatus::DeniedByUser == accessInfo->CurrentStatus)
        {
            rootPage->NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType::StatusMessage);
        }
        else if (DeviceAccessStatus::DeniedBySystem == accessInfo->CurrentStatus)
        {
            rootPage->NotifyUser("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", NotifyType::StatusMessage);
        }
        else if (DeviceAccessStatus::Unspecified == accessInfo->CurrentStatus)
        {
            rootPage->NotifyUser("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", NotifyType::StatusMessage);
        }
    }
    catch (Platform::Exception^ ex)
    {
        // GeofenceMonitor failed in adding a geofence
        // exceptions could be from out of memory, lat/long out of range,
        // too long a name, not a unique name, specifying a start
        // time + duration that is still in the past
        rootPage->NotifyUser(ex->ToString(), NotifyType::ErrorMessage);
    }
}

void Scenario4::FillRegisteredGeofenceListViewWithExistingGeofences()
{
    auto geofences = GeofenceMonitor::Current->Geofences;

    for each (auto geofence in geofences)
    {
        AddGeofenceToRegisteredGeofenceListView(geofence);
    }
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached. The Parameter
/// property is typically used to configure the page.</param>
void Scenario4::OnNavigatedTo(NavigationEventArgs^ e)
{
}

/// <summary>
/// Invoked immediately before the Page is unloaded and is no longer the current source of a parent Frame.
/// </summary>
/// <param name="e">
/// Event data that can be examined by overriding code. The event data is representative
/// of the navigation that will unload the current Page unless canceled. The
/// navigation can potentially be canceled by setting e.Cancel to true.
/// </param>
void Scenario4::OnNavigatedFrom(NavigationEventArgs^ e)
{
    if (true == inGetPositionAsync)
    {
        geopositionTaskTokenSource.cancel();
    }

    GeofenceMonitor::Current->GeofenceStateChanged::remove(geofencestatechangeToken);
    GeofenceMonitor::Current->StatusChanged::remove(geofencestatuschangeToken);
}

Platform::String^ Scenario4::GetTimeStampedMessage(Platform::String^ EventCalled)
{
    Platform::String^ message;

    Windows::Globalization::DateTimeFormatting::DateTimeFormatter^ formatter = ref new Windows::Globalization::DateTimeFormatting::DateTimeFormatter("longtime");
    Windows::Globalization::Calendar^ calendar = ref new Windows::Globalization::Calendar();
    calendar->SetToNow();

    message = EventCalled + " " + formatter->Format(calendar->GetDateTime());

    return message;
}

void Scenario4::OnAccessChangedHandler(DeviceAccessInformation^ sender, DeviceAccessChangedEventArgs^ args)
{
    // We need to dispatch to the UI thread to display the output
    Dispatcher->RunAsync(
        CoreDispatcherPriority::Normal,
        ref new DispatchedHandler(
        [this, args]()
    {
        Platform::String^ eventDescription = GetTimeStampedMessage("Device Access Status");

        if (DeviceAccessStatus::DeniedByUser == args->Status)
        {
            eventDescription += " (DeniedByUser)";

            geofenceEvents->InsertAt(0, eventDescription);

            rootPage->NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType::StatusMessage);
        }
        else if (DeviceAccessStatus::DeniedBySystem == args->Status)
        {
            eventDescription += " (DeniedBySystem)";

            geofenceEvents->InsertAt(0, eventDescription);

            rootPage->NotifyUser("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", NotifyType::StatusMessage);
        }
        else if (DeviceAccessStatus::Unspecified == args->Status)
        {
            eventDescription += " (Unspecified)";

            geofenceEvents->InsertAt(0, eventDescription);

            rootPage->NotifyUser("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", NotifyType::StatusMessage);
        }
        else if (DeviceAccessStatus::Allowed == args->Status)
        {
            eventDescription += " (Allowed)";

            geofenceEvents->InsertAt(0, eventDescription);

            // clear status
            rootPage->NotifyUser("", NotifyType::StatusMessage);

            if (false == geofenceStateChangedRegistered)
            {
                // register for state change events
                GeofenceMonitor::Current->GeofenceStateChanged += ref new TypedEventHandler<GeofenceMonitor^, Platform::Object^>(this, &SDKSample::GeolocationCPP::Scenario4::OnGeofenceStateChangedHandler);
                GeofenceMonitor::Current->StatusChanged += ref new TypedEventHandler<GeofenceMonitor^, Platform::Object^>(this, &SDKSample::GeolocationCPP::Scenario4::OnStatusChangedHandler);

                geofenceStateChangedRegistered = true;
            }
        }
        else
        {
            rootPage->NotifyUser("Unknown device access information status", NotifyType::StatusMessage);
        }
    },
    CallbackContext::Any
    )
    );
}

void Scenario4::OnStatusChangedHandler(GeofenceMonitor^ sender, Platform::Object^ args)
{
    auto status = sender->Status;

    Platform::String^ eventDescription = GetTimeStampedMessage("Geofence Status");

    if (GeofenceMonitorStatus::Ready == status)
    {
        eventDescription += " (Ready)";

        geofenceEvents->InsertAt(0, eventDescription);
    }
    else if (GeofenceMonitorStatus::Initializing == status)
    {
        eventDescription += " (Initializing)";

        geofenceEvents->InsertAt(0, eventDescription);
    }
    else if (GeofenceMonitorStatus::NoData == status)
    {
        eventDescription += " (NoData)";

        geofenceEvents->InsertAt(0, eventDescription);
    }
    else if (GeofenceMonitorStatus::Disabled == status)
    {
        eventDescription += " (Disabled)";

        geofenceEvents->InsertAt(0, eventDescription);
    }
    else if (GeofenceMonitorStatus::NotInitialized == status)
    {
        eventDescription += " (NotInitialized)";

        geofenceEvents->InsertAt(0, eventDescription);
    }
    else if (GeofenceMonitorStatus::NotAvailable == status)
    {
        eventDescription += " (NotAvailable)";

        geofenceEvents->InsertAt(0, eventDescription);
    }
}

void Scenario4::OnGeofenceStateChangedHandler(GeofenceMonitor^ sender, Platform::Object^ args)
{
    auto reports = sender->ReadReports();

    for each (auto report in reports)
    {
        GeofenceState state = report->NewState;

        auto geofence = report->Geofence;

        Platform::String^ eventDescription = GetTimeStampedMessage(geofence->Id);

        if (state == GeofenceState::Removed)
        {
            GeofenceRemovalReason reason = report->RemovalReason;

            if (reason == GeofenceRemovalReason::Expired)
            {
                eventDescription += " (Removed/Expired)";

                geofenceEvents->InsertAt(0, eventDescription);
            }
            else if (reason == GeofenceRemovalReason::Used)
            {
                eventDescription += " (Removed/Used)";

                geofenceEvents->InsertAt(0, eventDescription);
            }

            // remove this item from the collection
            auto itemToRemoveId = geofence->Id;

            // determine index at which itemToRemove is located
            unsigned int countInCollection = geofenceCollection->Size;

            bool foundInCollection = false;

            for (unsigned int loop = 0; loop < countInCollection; loop++)
            {
                GeofenceItem^ itemInCollection = safe_cast<GeofenceItem^>(geofenceCollection->GetAt(loop));

                auto itemInCollectionId = itemInCollection->Id;

                if (itemToRemoveId == itemInCollectionId)
                {
                    geofenceCollection->RemoveAt(loop);
                    foundInCollection = true;
                    break;
                }
            }

            if (false == foundInCollection)
            {
                auto strMsg = "Could not find GeofenceItem " + itemToRemoveId + " in geofenceCollection";

                rootPage->NotifyUser(strMsg, NotifyType::StatusMessage);
            }
        }
        else if (state == GeofenceState::Entered)
        {
            eventDescription += " (Entered)";

            geofenceEvents->InsertAt(0, eventDescription);
        }
        else if (state == GeofenceState::Exited)
        {
            eventDescription += " (Exited)";

            geofenceEvents->InsertAt(0, eventDescription);
        }
    }
}

void Scenario4::GeofenceDurationSecondTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Seconds", GeofenceDurationSecond);
}

void Scenario4::GeofenceDurationMinuteTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Minute", GeofenceDurationMinute);
}

void Scenario4::GeofenceDurationHourTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Hour", GeofenceDurationHour);
}

void Scenario4::GeofenceDurationDayTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Day", GeofenceDurationDay);
}

void Scenario4::GeofenceDwellTimeSecondTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Seconds", GeofenceDwellTimeSecond);
}

void Scenario4::GeofenceDwellTimeMinuteTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Minute", GeofenceDwellTimeMinute);
}

void Scenario4::GeofenceDwellTimeHourTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Hour", GeofenceDwellTimeHour);
}

void Scenario4::GeofenceDwellTimeDayTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Day", GeofenceDwellTimeDay);
}

void Scenario4::GeofenceLatitudeTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    latitudeSet = TextChangedHandlerDouble(false, "Latitude", GeofenceLatitude);

    CreateGeofenceButton->IsEnabled = SettingsAvailable();
}

void Scenario4::GeofenceLongitudeTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    longitudeSet = TextChangedHandlerDouble(false, "Longitude", GeofenceLongitude);

    CreateGeofenceButton->IsEnabled = SettingsAvailable();
}

void Scenario4::GeofenceRadiusTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    radiusSet = TextChangedHandlerDouble(false, "Radius", GeofenceRadius);

    CreateGeofenceButton->IsEnabled = SettingsAvailable();
}

void Scenario4::GeofenceNameTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    // get number of characters
    unsigned int charCount = GeofenceName->Text->Length();

    CharCount->Text = charCount.ToString() + " characters";
}

void Scenario4::GeofenceStartTimeSecondTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Seconds", GeofenceStartTimeSecond);
}

void Scenario4::GeofenceStartTimeMinuteTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Minute", GeofenceStartTimeMinute);
}

void Scenario4::GeofenceStartTimeHourTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Hour", GeofenceStartTimeHour);
}

void Scenario4::GeofenceStartTimeDayTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Day", GeofenceStartTimeDay);
}

void Scenario4::GeofenceStartTimeMonthTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Month", GeofenceStartTimeMonth);
}

void Scenario4::GeofenceStartTimeYearTextChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
    TextChangedHandlerInt(true, "Year", GeofenceStartTimeYear);
}

void Scenario4::GeofenceRegisteredListViewSelectionChangedHandler(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
    IVector<Object^>^ list = e->AddedItems;

    if (0 == list->Size)
    {
        // disable the remove button
        RemoveGeofenceItem->IsEnabled = false;
    }
    else
    {
        // enable the remove button
        RemoveGeofenceItem->IsEnabled = true;

        // update controls with the values from this geofence item
        // get selected item
        GeofenceItem^ item = safe_cast<GeofenceItem^>(GeofenceRegisteredListView->SelectedItem);

        RefreshControlsFromGeofenceItem(item);

        CreateGeofenceButton->IsEnabled = SettingsAvailable();

    }
}
        
void Scenario4::RefreshControlsFromGeofenceItem(GeofenceItem^ item)
{
    if (nullptr != item)
    {
        GeofenceName->Text = item->Id;
        GeofenceLatitude->Text = item->Latitude.ToString();
        GeofenceLongitude->Text = item->Longitude.ToString();
        GeofenceRadius->Text = item->Radius.ToString();

        GeofenceSingleUse->IsChecked = item->SingleUse;

        unsigned int states = item->MonitoredStates;
        unsigned int entered = static_cast<unsigned int>(MonitoredGeofenceStates::Entered);
        unsigned int exited = static_cast<unsigned int>(MonitoredGeofenceStates::Exited);
        unsigned int removed = static_cast<unsigned int>(MonitoredGeofenceStates::Removed);
        GeofenceEnter->IsChecked = ((states & entered) == entered);
        GeofenceExit->IsChecked = ((states & exited) == exited);
        GeofenceRemove->IsChecked = ((states & removed) == removed);

        ULARGE_INTEGER ull;

        SYSTEMTIME st = {};

        unsigned int totalSeconds;

        ull.QuadPart = item->Dwell;

        // safe to cast to unsigned int since days are limited to less than 999
        // which can safely be represented by unsigned int
        totalSeconds = static_cast<unsigned int>(ull.QuadPart/nanosecondsPerSecond);

        st.wDay = totalSeconds/secondsPerDay;

        if (maxDays < st.wDay)
        {
            st.wDay = maxDays;
        }

        totalSeconds -= st.wDay*secondsPerDay;

        st.wHour = totalSeconds/secondsPerHour;

        if (maxHours < st.wHour)
        {
            st.wHour = maxHours;
        }

        totalSeconds -= st.wHour*secondsPerHour;

        st.wMinute = totalSeconds/secondsPerMinute;

        if (maxMinutes < st.wMinute)
        {
            st.wMinute = maxMinutes;
        }

        totalSeconds -= st.wMinute*secondsPerMinute;

        st.wSecond = totalSeconds;

        if (0 != st.wDay)
        {
            GeofenceDwellTimeDay->Text = st.wDay.ToString();
        }
        else
        {
            GeofenceDwellTimeDay->Text = "";
        }
        if (0 != st.wHour)
        {
            GeofenceDwellTimeHour->Text = st.wHour.ToString();
        }
        else
        {
            GeofenceDwellTimeHour->Text = "";
        }
        if (0 != st.wMinute)
        {
            GeofenceDwellTimeMinute->Text = st.wMinute.ToString();
        }
        else
        {
            GeofenceDwellTimeMinute->Text = "";
        }
        if (0 != st.wSecond)
        {
            GeofenceDwellTimeSecond->Text = st.wSecond.ToString();
        }
        else
        {
            GeofenceDwellTimeSecond->Text = "";
        }

        ull.QuadPart = item->Duration;

        // safe to cast to unsigned int since days are limited to less than 999
        // which can safely be represented by unsigned int
        totalSeconds = static_cast<unsigned int>(ull.QuadPart/nanosecondsPerSecond);

        st.wDay = totalSeconds/secondsPerDay;

        if (maxDays < st.wDay)
        {
            st.wDay = maxDays;
        }

        totalSeconds -= st.wDay*secondsPerDay;

        st.wHour = totalSeconds/secondsPerHour;

        if (maxHours < st.wHour)
        {
            st.wHour = maxHours;
        }

        totalSeconds -= st.wHour*secondsPerHour;

        st.wMinute = totalSeconds/secondsPerMinute;

        if (maxMinutes < st.wMinute)
        {
            st.wMinute = maxMinutes;
        }

        totalSeconds -= st.wMinute*secondsPerMinute;

        st.wSecond = totalSeconds;

        if (0 != st.wDay)
        {
            GeofenceDurationDay->Text = st.wDay.ToString();
        }
        else
        {
            GeofenceDurationDay->Text = "";
        }
        if (0 != st.wHour)
        {
            GeofenceDurationHour->Text = st.wHour.ToString();
        }
        else
        {
            GeofenceDurationHour->Text = "";
        }
        if (0 != st.wMinute)
        {
            GeofenceDurationMinute->Text = st.wMinute.ToString();
        }
        else
        {
            GeofenceDurationMinute->Text = "";
        }
        if (0 != st.wSecond)
        {
            GeofenceDurationSecond->Text = st.wSecond.ToString();
        }
        else
        {
            GeofenceDurationSecond->Text = "";
        }

        FILETIME ft = {};

        ull.QuadPart = item->Start;

        ft.dwHighDateTime = ull.HighPart;
        ft.dwLowDateTime = ull.LowPart;

        if (((0 != ft.dwHighDateTime) || (0 != ft.dwLowDateTime)) && (TRUE == FileTimeToSystemTime(&ft, &st)))
        {
            if (0 != st.wYear)
            {
                GeofenceStartTimeYear->Text = st.wYear.ToString();
            }
            else
            {
                GeofenceStartTimeYear->Text = "";
            }
            if (0 != st.wMonth)
            {
                GeofenceStartTimeMonth->Text = st.wMonth.ToString();
            }
            else
            {
                GeofenceStartTimeMonth->Text = "";
            }
            if (0 != st.wDay)
            {
                GeofenceStartTimeDay->Text = st.wDay.ToString();
            }
            else
            {
                GeofenceStartTimeDay->Text = "";
            }
            if (0 != st.wHour)
            {
                GeofenceStartTimeHour->Text = st.wHour.ToString();
            }
            else
            {
                GeofenceStartTimeHour->Text = "";
            }
            if (0 != st.wMinute)
            {
                GeofenceStartTimeMinute->Text = st.wMinute.ToString();
            }
            else
            {
                GeofenceStartTimeMinute->Text = "";
            }
            if (0 != st.wSecond)
            {
                GeofenceStartTimeSecond->Text = st.wSecond.ToString();
            }
            else
            {
                GeofenceStartTimeSecond->Text = "";
            }
        }
        else
        {
            GeofenceStartTimeYear->Text = "";
            GeofenceStartTimeMonth->Text = "";
            GeofenceStartTimeDay->Text = "";
            GeofenceStartTimeHour->Text = "";
            GeofenceStartTimeMinute->Text = "";
            GeofenceStartTimeSecond->Text = "";
        }

        // Update flags used to enable Create Geofence button
        GeofenceNameTextChangedHandler(nullptr, nullptr);
        GeofenceLongitudeTextChangedHandler(nullptr, nullptr);
        GeofenceLatitudeTextChangedHandler(nullptr, nullptr);
        GeofenceRadiusTextChangedHandler(nullptr, nullptr);
    }
}

bool Scenario4::TextChangedHandlerDouble(bool nullAllowed, Platform::String^ name, Windows::UI::Xaml::Controls::TextBox^ e)
{
    bool valueSet = false;

    if (e->Text->IsEmpty())
    {
        if (false == nullAllowed)
        {
            if (nullptr != name)
            {
                auto strMsg = name + " needs a value";

                rootPage->NotifyUser(strMsg, NotifyType::StatusMessage);
            }
        }
        else
        {
            valueSet = true;
        }
    }
    else
    {
        valueSet = true;

        double value = FromStringTo<double>(e->Text);

        if (0 == value)
        {
            // make sure string was '0'
            if ("0" != e->Text)
            {
                auto strMsg = name + " must be a number";

                rootPage->NotifyUser(strMsg, NotifyType::StatusMessage);

                valueSet = false;
            }
        }

        if (true == valueSet)
        {
            // clear out status message
            rootPage->NotifyUser("", NotifyType::StatusMessage);
        }
    }

    return valueSet;
}

bool Scenario4::TextChangedHandlerInt(bool nullAllowed, Platform::String^ name, Windows::UI::Xaml::Controls::TextBox^ e)
{
    bool valueSet = false;

    if (e->Text->IsEmpty())
    {
        if (false == nullAllowed)
        {
            if (nullptr != name)
            {
                auto strMsg = name + " needs a value";

                rootPage->NotifyUser(strMsg, NotifyType::StatusMessage);
            }
        }
        else
        {
            valueSet = true;
        }
    }
    else
    {
        valueSet = true;

        int value = FromStringTo<int>(e->Text);

        if (0 == value)
        {
            // make sure string was '0'
            if ("0" != e->Text)
            {
                auto strMsg = name + " must be a number";

                rootPage->NotifyUser(strMsg, NotifyType::StatusMessage);

                valueSet = false;
            }
        }

        if (true == valueSet)
        {
            // clear out status message
            rootPage->NotifyUser("", NotifyType::StatusMessage);
        }
    }

    return valueSet;
}

// are settings available so a geofence can be created?
bool Scenario4::SettingsAvailable()
{
    bool fSettingsAvailable = false;

    if ((true == latitudeSet) &&
        (true == longitudeSet) &&
        (true == radiusSet) )
    {
        // also need to test if data is good
        fSettingsAvailable = true;
    }

    return fSettingsAvailable;
}

// add geofence to listview
void Scenario4::AddGeofenceToRegisteredGeofenceListView(Windows::Devices::Geolocation::Geofencing::Geofence^ geofence)
{
    GeofenceItem^ item = ref new GeofenceItem(geofence);

    // The registered geofence listview is data bound
    // to the collection in the root page
    // so once the GeofenceItem is added to the 
    // collection it will appear in the 
    // registered geofence listview.
    geofenceCollection->InsertAt(0, item);
}

/// <summary>
/// This is the click handler for the 'Remove Geofence Item' button.
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void Scenario4::Remove(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        if (nullptr != GeofenceRegisteredListView->SelectedItem)
        {
            // get selected item
            GeofenceItem^ itemToRemove = safe_cast<GeofenceItem^>(GeofenceRegisteredListView->SelectedItem);

            auto geofence = itemToRemove->Geofence;

            auto geofences = GeofenceMonitor::Current->Geofences;

            unsigned int index;

            if (geofences->IndexOf(geofence, &index))
            {
                geofences->RemoveAt(index);
            }
            else
            {
                auto strMsg = "Could not find GeofenceItem " + geofence->Id + " in GeofenceMonitor";

                rootPage->NotifyUser(strMsg, NotifyType::StatusMessage);
            }

            if (geofenceCollection->IndexOf(itemToRemove, &index))
            {
                geofenceCollection->RemoveAt(index);
            }
            else
            {
                auto strMsg = "Could not find GeofenceItem " + itemToRemove->Id + " in geofenceCollection";

                rootPage->NotifyUser(strMsg, NotifyType::StatusMessage);
            }
        }
    }
    catch (Platform::Exception^ ex)
    {
        rootPage->NotifyUser(ex->ToString(), NotifyType::ErrorMessage);
    }
}

Windows::Devices::Geolocation::Geofencing::Geofence^ Scenario4::GenerateGeofence()
{
    Geofence^ geofence = nullptr;

    try
    {
        String^ fenceKey = GeofenceName->Text;

        BasicGeoposition position;
        position.Latitude = FromStringTo<double>(GeofenceLatitude->Text);
        position.Longitude = FromStringTo<double>(GeofenceLongitude->Text);
        position.Altitude = 0.0;
        double radius = FromStringTo<double>(GeofenceRadius->Text);

        // the geofence is a circular region
        Geocircle^ geocircle = ref new Geocircle(position, radius);

        bool singleUse = false;

        if (true == GeofenceSingleUse->IsChecked->Value)
        {
            singleUse = true;
        }

        // want to listen for enter geofence, exit geofence and remove geofence events
        // you can select a subset of these event states
        MonitoredGeofenceStates mask = static_cast<MonitoredGeofenceStates>(0);

        if (true == GeofenceEnter->IsChecked->Value)
        {
            mask = mask | MonitoredGeofenceStates::Entered;
        }
        if (true == GeofenceExit->IsChecked->Value)
        {
            mask = mask | MonitoredGeofenceStates::Exited;
        }
        if (true == GeofenceRemove->IsChecked->Value)
        {
            mask = mask | MonitoredGeofenceStates::Removed;
        }

        // setting up how long you need to be in geofence for enter event to fire
        int dwellTimeDays = 0;
        int dwellTimeHours = 0;
        int dwellTimeMinutes = 0;
        int dwellTimeSeconds = 0;

        bool useDwellTime = false;

        // use dwell if at least one textbox has text
        if ((nullptr != GeofenceDwellTimeDay->Text) ||
            (nullptr != GeofenceDwellTimeHour->Text) ||
            (nullptr != GeofenceDwellTimeMinute->Text) ||
            (nullptr != GeofenceDwellTimeSecond->Text))
        {
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceDwellTimeDay))
            {
                dwellTimeDays = FromStringTo<int>(GeofenceDwellTimeDay->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceDwellTimeHour))
            {
                dwellTimeHours = FromStringTo<int>(GeofenceDwellTimeHour->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceDwellTimeMinute))
            {
                dwellTimeMinutes = FromStringTo<int>(GeofenceDwellTimeMinute->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceDwellTimeSecond))
            {
                dwellTimeSeconds = FromStringTo<int>(GeofenceDwellTimeSecond->Text);
            }
        }

        if ((0 != dwellTimeDays) || (0 != dwellTimeHours) || (0 != dwellTimeMinutes) || (0 != dwellTimeSeconds))
        {
            useDwellTime = true;
        }

        // setting up how long the geofence should be active
        int durationDays = 0;
        int durationHours = 0;
        int durationMinutes = 0;
        int durationSeconds = 0;

        bool useDuration = false;

        // use duration if at least one textbox has text
        if ((nullptr != GeofenceDurationDay->Text) ||
            (nullptr != GeofenceDurationHour->Text) ||
            (nullptr != GeofenceDurationMinute->Text) ||
            (nullptr != GeofenceDurationSecond->Text))
        {
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceDurationDay))
            {
                durationDays = FromStringTo<int>(GeofenceDurationDay->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceDurationHour))
            {
                durationHours = FromStringTo<int>(GeofenceDurationHour->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceDurationMinute))
            {
                durationMinutes = FromStringTo<int>(GeofenceDurationMinute->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceDurationSecond))
            {
                durationSeconds = FromStringTo<int>(GeofenceDurationSecond->Text);
            }
        }

        if ((0 != durationDays) || (0 != durationHours) || (0 != durationMinutes) || (0 != durationSeconds))
        {
            useDuration = true;
        }

        // setting up the start time of the geofence
        int startTimeYear = 0;
        int startTimeMonth = 0;
        int startTimeDay = 0;
        int startTimeHour = 0;
        int startTimeMinute = 0;
        int startTimeSecond = 0;

        bool useStartTime = false;

        // use duration if at least one textbox has text
        if ((nullptr != GeofenceStartTimeYear->Text) ||
            (nullptr != GeofenceStartTimeMonth->Text) ||
            (nullptr != GeofenceStartTimeDay->Text) ||
            (nullptr != GeofenceStartTimeHour->Text) ||
            (nullptr != GeofenceStartTimeMinute->Text) ||
            (nullptr != GeofenceStartTimeSecond->Text))
        {
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceStartTimeYear))
            {
                startTimeYear = FromStringTo<int>(GeofenceStartTimeYear->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceStartTimeMonth))
            {
                startTimeMonth = FromStringTo<int>(GeofenceStartTimeMonth->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceStartTimeDay))
            {
                startTimeDay = FromStringTo<int>(GeofenceStartTimeDay->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceStartTimeHour))
            {
                startTimeHour = FromStringTo<int>(GeofenceStartTimeHour->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceStartTimeMinute))
            {
                startTimeMinute = FromStringTo<int>(GeofenceStartTimeMinute->Text);
            }
            if (true == TextChangedHandlerInt(true, nullptr, GeofenceStartTimeSecond))
            {
                startTimeSecond = FromStringTo<int>(GeofenceStartTimeSecond->Text);
            }
        }

        if ((0 != startTimeYear) || (0 != startTimeMonth) || (0 != startTimeDay) || (0 != startTimeHour) || (0 != startTimeMinute) || (0 != startTimeSecond))
        {
            useStartTime = true;
        }

        if ((false == useStartTime) && (false == useDuration))
        {
            if (true == useDwellTime)
            {
                // mask must be set, so set it to default values
                if (static_cast<MonitoredGeofenceStates>(0) == mask)
                {
                    mask = MonitoredGeofenceStates::Entered | MonitoredGeofenceStates::Exited;
                }

                TimeSpan dwellTime;
                // need to set duration expressed in 100-nanoseconds
                // note that this is FILETIME units
                // start with SYSTEMTIME and convert to FILETIME

                // count number of seconds
                unsigned int totalSeconds = dwellTimeSeconds + dwellTimeMinutes*secondsPerMinute + dwellTimeHours*secondsPerHour + dwellTimeDays*secondsPerDay;

                // convert to 100-nanoseconds
                ULARGE_INTEGER ull;

                ull.QuadPart = totalSeconds;

                ull.QuadPart *= nanosecondsPerSecond;

                dwellTime.Duration = ull.QuadPart;

                geofence = ref new Geofence(fenceKey, geocircle, mask, singleUse, dwellTime);
            }
            else
            {
                if (static_cast<MonitoredGeofenceStates>(0) != mask)
                {
                    geofence = ref new Geofence(fenceKey, geocircle, mask, singleUse);
                }
                else
                {
                    geofence = ref new Geofence(fenceKey, geocircle);
                }
            }
        }
        else
        {
            // mask must be set, so set it to default values
            if (static_cast<MonitoredGeofenceStates>(0) == mask)
            {
                mask = MonitoredGeofenceStates::Entered | MonitoredGeofenceStates::Exited;
            }

            // need to set duration expressed in 100-nanoseconds
            // note that this is FILETIME units
            // start with SYSTEMTIME and convert to FILETIME

            TimeSpan dwellTime;
            // count number of seconds
            unsigned int totalSeconds = dwellTimeSeconds + dwellTimeMinutes*secondsPerMinute + dwellTimeHours*secondsPerHour + dwellTimeDays*secondsPerDay;

            // convert to 100-nanoseconds
            ULARGE_INTEGER ull;

            ull.QuadPart = totalSeconds;

            ull.QuadPart *= nanosecondsPerSecond;

            dwellTime.Duration = ull.QuadPart;

            TimeSpan duration;
            // count number of seconds
            totalSeconds = durationSeconds + durationMinutes*secondsPerMinute + durationHours*secondsPerHour + durationDays*secondsPerDay;

            // convert to 100-nanoseconds
            ull.QuadPart = totalSeconds;

            ull.QuadPart *= nanosecondsPerSecond;

            duration.Duration = ull.QuadPart;

            SYSTEMTIME st = {};
            FILETIME ft = {};

            DateTime startTime;
            st.wYear = startTimeYear;
            st.wMonth = startTimeMonth;
            st.wDay = startTimeDay;
            st.wHour = startTimeHour;
            st.wMinute = startTimeMinute;
            st.wSecond = startTimeSecond;
            SystemTimeToFileTime(&st, &ft);

            ull.HighPart = ft.dwHighDateTime;
            ull.LowPart = ft.dwLowDateTime;

            startTime.UniversalTime = ull.QuadPart;

            geofence = ref new Geofence(fenceKey, geocircle, mask, singleUse, dwellTime, startTime, duration);
        }
    }
    catch (Platform::Exception^ ex)
    {
        // GeofenceMonitor failed in adding a geofence
        // exceptions could be from out of memory, lat/long out of range,
        // too long a name, not a unique name, specifying an activation
        // time + duration that is still in the past
        rootPage->NotifyUser(ex->ToString(), NotifyType::ErrorMessage);
    }

    return geofence;
}

/// <summary>
/// This is the click handler for the 'Create Geofence' button.
/// </summary>
/// <param name="sender"></param>
/// <param name="e"></param>
void Scenario4::CreateGeofence(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    try
    {
        // This must be done here because there is no guarantee of 
        // getting the location consent from a geofence call.
        if (false == permissionsChecked)
        {
            GetGeopositionAsync();
            permissionsChecked = true;
        }

        // get lat/long/radius, the fence name (fenceKey), 
        // and other properties from controls,
        // depending on data in controls for activation time
        // and duration the appropriate
        // constructor will be used.
        Geofence^ geofence = GenerateGeofence();

        // Add the geofence to the GeofenceMonitor's
        // collection of fences
        GeofenceMonitor::Current->Geofences->InsertAt(0, geofence);

        // add geofence to listview
        AddGeofenceToRegisteredGeofenceListView(geofence);
    }
    catch (Platform::AccessDeniedException^)
    {
        if (DeviceAccessStatus::DeniedByUser == accessInfo->CurrentStatus)
        {
            rootPage->NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType::StatusMessage);
        }
        else if (DeviceAccessStatus::DeniedBySystem == accessInfo->CurrentStatus)
        {
            rootPage->NotifyUser("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", NotifyType::StatusMessage);
        }
        else if (DeviceAccessStatus::Unspecified == accessInfo->CurrentStatus)
        {
            rootPage->NotifyUser("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", NotifyType::StatusMessage);
        }
    }
    catch (task_canceled&)
    {
        rootPage->NotifyUser("Canceled", NotifyType::StatusMessage);
    }
    catch (Platform::Exception^ ex)
    {
        // GeofenceMonitor failed in adding a geofence
        // exceptions could be from out of memory, lat/long out of range,
        // too long a name, not a unique name, specifying an activation
        // time + duration that is still in the past
        rootPage->NotifyUser(ex->ToString(), NotifyType::ErrorMessage);
    }
}

void Scenario4::GetGeopositionAsync()
{
    rootPage->NotifyUser("Checking permissions...", NotifyType::StatusMessage);

    inGetPositionAsync = true;

    task<Geoposition^> geopositionTask(geolocator->GetGeopositionAsync(), geopositionTaskTokenSource.get_token());
    geopositionTask.then([this](task<Geoposition^> getPosTask)
    {
        try
        {
            // Get will throw an exception if the task was canceled or failed with an error
            Geoposition^ pos = getPosTask.get();

            // clear status
            rootPage->NotifyUser("", NotifyType::StatusMessage);
        }
        catch (AccessDeniedException^)
        {
            if (DeviceAccessStatus::DeniedByUser == accessInfo->CurrentStatus)
            {
                rootPage->NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType::StatusMessage);
            }
            else if (DeviceAccessStatus::DeniedBySystem == accessInfo->CurrentStatus)
            {
                rootPage->NotifyUser("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", NotifyType::StatusMessage);
            }
            else if (DeviceAccessStatus::Unspecified == accessInfo->CurrentStatus)
            {
                rootPage->NotifyUser("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", NotifyType::StatusMessage);
            }
        }
        catch (task_canceled&)
        {
            rootPage->NotifyUser("Operation canceled", NotifyType::StatusMessage);
        }
        catch (Exception^ ex)
        {
            // If there are no location sensors GetGeopositionAsync()
            // will timeout -- that is acceptable.

            if (ex->HResult == HRESULT_FROM_WIN32(WAIT_TIMEOUT))
            {
                rootPage->NotifyUser("Operation accessing location sensors timed out. Possibly there are no location sensors.", NotifyType::StatusMessage);
            }
            else
            {
                rootPage->NotifyUser(ex->ToString(), NotifyType::ErrorMessage);
            }
        }
    });

    inGetPositionAsync = false;
}

Windows::Devices::Geolocation::Geofencing::Geofence^ GeofenceItem::Geofence::get()
{
    return geofence;
}

Platform::String^ GeofenceItem::Id::get()
{
    return geofence->Id;
}

double GeofenceItem::Latitude::get()
{
    Geocircle^ circle = safe_cast<Geocircle^> (geofence->Geoshape);
    return circle->Center.Latitude;
}

double GeofenceItem::Longitude::get()
{
    Geocircle^ circle = safe_cast<Geocircle^> (geofence->Geoshape);
    return circle->Center.Longitude;
}

double GeofenceItem::Radius::get()
{
    Geocircle^ circle = safe_cast<Geocircle^> (geofence->Geoshape);
    return circle->Radius;
}

long long GeofenceItem::Dwell::get()
{
    return geofence->DwellTime.Duration;
}

bool GeofenceItem::SingleUse::get()
{
    return geofence->SingleUse;
}

unsigned int GeofenceItem::MonitoredStates::get()
{
    return safe_cast<int>(geofence->MonitoredStates);
}

long long GeofenceItem::Duration::get()
{
    return geofence->Duration.Duration;
}

long long GeofenceItem::Start::get()
{
    return geofence->StartTime.UniversalTime;
}
