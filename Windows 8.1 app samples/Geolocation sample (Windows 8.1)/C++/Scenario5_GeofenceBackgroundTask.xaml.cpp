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
// Scenario5_BackgroundTask.xaml.cpp
// Implementation of the Scenario5 class
//

#include "pch.h"
#include "Scenario5_GeofenceBackgroundTask.xaml.h"

using namespace SDKSample;
using namespace SDKSample::GeolocationCPP;

using namespace concurrency;
using namespace Platform;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Data::Json;
using namespace Windows::Devices::Geolocation;
using namespace Windows::Foundation;
using namespace Windows::Storage;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::UI::Notifications;
using namespace Windows::Data::Xml::Dom;
using namespace Windows::Globalization::DateTimeFormatting;

const int c_datetimeResolutionToSeconds = 10000000;  // conversion from 100 nano-second resolution to seconds
const unsigned int c_minEventArraySize = 10;		// minimum size of Json array containing date/latitude/longitude + event data

Scenario5::Scenario5() : rootPage(MainPage::Current), sampleBackgroundTaskName("SampleGeofenceBackgroundTask"), sampleBackgroundTaskEntryPoint("BackgroundTask.GeofenceBackgroundTask")
{
    InitializeComponent();

    auto settings = ApplicationData::Current->LocalSettings->Values;
    if (settings->HasKey("GeofenceEvent"))
    {
        settings->Remove("GeofenceEvent");
    }

    geolocator = ref new Geolocator();

    geofenceBackgroundEvents = ref new Platform::Collections::Vector<Object^>();

    // using data binding to the root page collection of GeofenceItems associated with events
    GeofenceBackgroundEventsListView->DataContext = geofenceBackgroundEvents;
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>
void Scenario5::OnNavigatedTo(NavigationEventArgs^ e)
{
    // Look for an already registered background task
    geofenceTask = nullptr;

    auto iter = BackgroundTaskRegistration::AllTasks->First();
    while (iter->HasCurrent)
    {
        auto task = iter->Current;
        if (task->Value->Name == sampleBackgroundTaskName)
        {
            geofenceTask = safe_cast<BackgroundTaskRegistration^>(task->Value);
            break;
        }
        iter->MoveNext();
    }

    if (geofenceTask != nullptr)
    {
        // Register for background task completion notifications
        taskCompletedToken = geofenceTask->Completed::add(ref new BackgroundTaskCompletedEventHandler(this, &Scenario5::OnCompleted));

        // Check the background access status of the application and display the appropriate status message
        switch (BackgroundExecutionManager::GetAccessStatus())
        {
        case BackgroundAccessStatus::Unspecified:
        case BackgroundAccessStatus::Denied:
            rootPage->NotifyUser("This application must be added to the lock screen before the background task will run.", NotifyType::ErrorMessage);
            break;

        default:
            rootPage->NotifyUser("Background task is already registered. Waiting for next update...", NotifyType::StatusMessage);
            break;
        }

        RegisterBackgroundTaskButton->IsEnabled = false;
        UnregisterBackgroundTaskButton->IsEnabled = true;
    }
    else
    {
        RegisterBackgroundTaskButton->IsEnabled = true;
        UnregisterBackgroundTaskButton->IsEnabled = false;
    }
}

/// <summary>
/// Invoked when this page is no longer displayed.
/// </summary>
/// <param name="e"></param>
void Scenario5::OnNavigatedFrom(NavigationEventArgs^ e)
{
    // Just in case the original GetGeopositionAsync call is still active
    geopositionTaskTokenSource.cancel();

    if (geofenceTask != nullptr)
    {
        geofenceTask->Completed::remove(taskCompletedToken);
    }
}

void Scenario5::RegisterBackgroundTask(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // Get permission for a background task from the user. If the user has already answered once,
    // this does nothing and the user must manually update their preference via PC Settings.
    task<BackgroundAccessStatus> requestAccessTask(BackgroundExecutionManager::RequestAccessAsync());
    requestAccessTask.then([this](BackgroundAccessStatus backgroundAccessStatus)
    {
        // Regardless of the answer, register the background task. If the user later adds this application
        // to the lock screen, the background task will be ready to run.

        // Create a new background task builder
        BackgroundTaskBuilder^ geofenceTaskBuilder = ref new BackgroundTaskBuilder();

        geofenceTaskBuilder->Name = sampleBackgroundTaskName;
        geofenceTaskBuilder->TaskEntryPoint = sampleBackgroundTaskEntryPoint;

        // Create a new location trigger
        auto trigger = ref new LocationTrigger(LocationTriggerType::Geofence);

        // Associate the location trigger with the background task builder
        geofenceTaskBuilder->SetTrigger(trigger);

        // If it is important that there is user presence and/or
        // internet connection when OnCompleted is called
        // the following could be called before calling Register()
        // SystemCondition^ condition = ref new SystemCondition(SystemConditionType::UserPresent | SystemConditionType::InternetAvailable);
        // geofenceTaskBuilder->AddCondition(condition);

        // Register the background task
        geofenceTask = geofenceTaskBuilder->Register();

        // Register for background task completion notifications
        taskCompletedToken = geofenceTask->Completed::add(ref new BackgroundTaskCompletedEventHandler(this, &Scenario5::OnCompleted));

        RegisterBackgroundTaskButton->IsEnabled = false;
        UnregisterBackgroundTaskButton->IsEnabled = true;

        // Check the background access status of the application and display the appropriate status message
        switch (backgroundAccessStatus)
        {
        case BackgroundAccessStatus::Unspecified:
        case BackgroundAccessStatus::Denied:
            rootPage->NotifyUser("This application must be added to the lock screen before the background task will run.", NotifyType::ErrorMessage);
            break;

        default:
            GetGeopositionAsync();
            break;
        }
    });
}

void Scenario5::UnregisterBackgroundTask(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    // Remove the application from the lock screen
    BackgroundExecutionManager::RemoveAccess();

    // Unregister the background task
    if (geofenceTask != nullptr)
    {
        geofenceTask->Unregister(true);
        geofenceTask = nullptr;
    }

    rootPage->NotifyUser("Background task unregistered", NotifyType::StatusMessage);

    RegisterBackgroundTaskButton->IsEnabled = true;
    UnregisterBackgroundTaskButton->IsEnabled = false;
}

void Scenario5::OnCompleted(BackgroundTaskRegistration^ task, Windows::ApplicationModel::Background::BackgroundTaskCompletedEventArgs^ args)
{
    // Update the UI with progress reported by the background task
    // We need to dispatch to the UI thread to display the output
    Dispatcher->RunAsync(
        CoreDispatcherPriority::Normal,
        ref new DispatchedHandler(
            [this, args]()
            {
                try
                {
                    // Throw an exception if the background task had an unrecoverable error
                    args->CheckResult();

                    // This method waits for the current
                    // position to be acquired for comparison
                    // to the latitude & longitude last tracked 
                    // by the GeofenceMonitor when the background
                    // event took place.
                    //
                    // If comparison of location isn't important
                    // the code in UpdateUIWithPosition() could
                    // be placed here.
                    //
                    // Also, if it were important that there 
                    // be user presence and/or internet
                    // connection when OnComplete is called
                    // the background task can be 
                    // registered with those conditions.
                    // See note in RegisterBackgroundTask()
                    // on how to do this.
                    UpdateUIAfterPositionFix();
                }
                catch (Exception^ ex)
                {
                    // The background task had an error
                    rootPage->NotifyUser(ex->Message, NotifyType::ErrorMessage);
                }
            },
            CallbackContext::Any
            )
        );
}

void Scenario5::GetGeopositionAsync()
{
    rootPage->NotifyUser("Checking permissions...", NotifyType::StatusMessage);

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
            rootPage->NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType::ErrorMessage);
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
}

void Scenario5::UpdateUIAfterPositionFix()
{
    task<Geoposition^> geopositionTask(geolocator->GetGeopositionAsync(), geopositionTaskTokenSource.get_token());
    geopositionTask.then([this](task<Geoposition^> getPosTask)
    {
        try
        {
            // Get will throw an exception if the task was canceled or failed with an error
            Geoposition^ pos = getPosTask.get();

            UpdateUIWithPosition(pos);
        }
        catch (AccessDeniedException^)
        {
            rootPage->NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType::ErrorMessage);
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
}

void Scenario5::UpdateUIWithPosition(Geoposition^ pos)
{
    // Update the UI with the completion status of the background task
    auto settings = ApplicationData::Current->LocalSettings->Values;
    if (settings->HasKey("Status"))
    {
        rootPage->NotifyUser(safe_cast<String^>(settings->Lookup("Status")), NotifyType::StatusMessage);
    }

    // pop a toast for each geofence event
    // and add to listview
    if (settings->HasKey("GeofenceEvent"))
    {
        String^ geofenceEvent = safe_cast<String^>(settings->Lookup("GeofenceEvent"));

        if (0 != geofenceEvent->Length())
        {
            JsonValue^ jsonValue = JsonValue::Parse(geofenceEvent);
            unsigned int arraySize = jsonValue->GetArray()->Size;

            IJsonValue^ element = nullptr;
            // the array contains
            // year
            // month
            // day
            // hour
            // minute
            // second
            // period (1:AM, 2:PM)
            // latitude
            // longitude

            // then for each geofence event
            // geofence.id
            // GeofenceReason (as double)

            enum BackgroundData { Year = 0, Month, Day, Hour, Minute, Second, Period, Latitude, Longitude, FirstEventName };

            // so minumum size of array should be 10
            if (arraySize >= c_minEventArraySize)
            {
                Windows::Globalization::Calendar^ calendar = ref new Windows::Globalization::Calendar();
                bool eventOfInterest = true;

                element = jsonValue->GetArray()->GetAt(BackgroundData::Year);
                calendar->Year = safe_cast<int>(element->GetNumber());
                element = jsonValue->GetArray()->GetAt(BackgroundData::Month);
                calendar->Month = safe_cast<int>(element->GetNumber());
                element = jsonValue->GetArray()->GetAt(BackgroundData::Day);
                calendar->Day = safe_cast<int>(element->GetNumber());
                element = jsonValue->GetArray()->GetAt(BackgroundData::Hour);
                calendar->Hour = safe_cast<int>(element->GetNumber());
                element = jsonValue->GetArray()->GetAt(BackgroundData::Minute);
                calendar->Minute = safe_cast<int>(element->GetNumber());
                element = jsonValue->GetArray()->GetAt(BackgroundData::Second);
                calendar->Second = safe_cast<int>(element->GetNumber());
                element = jsonValue->GetArray()->GetAt(BackgroundData::Period);
                calendar->Period = safe_cast<int>(element->GetNumber());

                // NOTE TO DEVELOPER:
                // This event can be filtered out if the
                // geofence event location is stale.
                DateTime eventDateTime = calendar->GetDateTime();

                calendar->SetToNow();
                DateTime nowDateTime = calendar->GetDateTime();

                DateTime diffDateTime;
                diffDateTime.UniversalTime = nowDateTime.UniversalTime - eventDateTime.UniversalTime;

                long long deltaInSeconds = diffDateTime.UniversalTime / c_datetimeResolutionToSeconds;

                // NOTE TO DEVELOPER:
                // If the time difference between the geofence event and now is too large
                // the eventOfInterest should be set to false.

                String^ geofenceItemEvent = nullptr;

                int numEventsOfInterest = (arraySize - BackgroundData::FirstEventName)/2;

                if (eventOfInterest)
                {
                    element = jsonValue->GetArray()->GetAt(BackgroundData::Latitude);
                    double latitudeEvent = element->GetNumber();
                    element = jsonValue->GetArray()->GetAt(BackgroundData::Longitude);
                    double longitudeEvent = element->GetNumber();

                    // NOTE TO DEVELOPER:
                    // This event can be filtered out if the
                    // geofence event location is too far away.
                    if ((latitudeEvent != pos->Coordinate->Point->Position.Latitude) ||
                        (longitudeEvent != pos->Coordinate->Point->Position.Longitude))
                    {
                        // NOTE TO DEVELOPER:
                        // Use an algorithm like Haversine to determine
                        // the distance between the current location (pos->Coordinate)
                        // and the location of the geofence event (latitudeEvent/longitudeEvent).
                        // If too far apart set eventOfInterest to false to
                        // filter the event out.
                    }

                    if (eventOfInterest)
                    {
                        for (unsigned int loop = BackgroundData::FirstEventName; loop < arraySize; )
                        {
                            geofenceItemEvent = jsonValue->GetArray()->GetAt(loop++)->GetString();
                            int reason = safe_cast<int>(jsonValue->GetArray()->GetAt(loop++)->GetNumber());

                            switch (reason)
                            {
                            case 0: // GeofenceReason::Add:
                                geofenceItemEvent += " (Added)";
                                break;

                            case 1: // GeofenceReason::Entered:
                                geofenceItemEvent += " (Entered)";
                                break;

                            case 2: // GeofenceReason::Exited:
                                geofenceItemEvent += " (Exited)";
                                break;

                            case 3: // GeofenceReason::Expired:
                                geofenceItemEvent += " (Removed/Expired)";
                                break;

                            case 4: // GeofenceReason::Used:
                                geofenceItemEvent += " (Removed/Used)";
                                break;

                            default:
                                break;
                            }

                            if (0 != geofenceItemEvent->Length())
                            {
                                // now add event to listview
                                geofenceBackgroundEvents->InsertAt(0, geofenceItemEvent);
                            }
                            else
                            {
                                --numEventsOfInterest;
                            }
                        }
                    }
                }

                if (settings->HasKey("GeofenceEvent"))
                {
                    settings->Remove("GeofenceEvent");
                }

                if (eventOfInterest && (0 != numEventsOfInterest))
                {
                    DoToast(numEventsOfInterest, geofenceItemEvent);
                    DoTile(numEventsOfInterest, geofenceItemEvent);
                    DoBadge(numEventsOfInterest);
                }
            }
        }
    }
}

// Helper method to pop a toast
void Scenario5::DoToast(int numEventsOfInterest, Platform::String^ eventName)
{
    // pop a toast for each geofence event
    ToastNotifier^ ToastNotifier = ToastNotificationManager::CreateToastNotifier();

    // Create a two line toast and add audio reminder

    // Here the xml that will be passed to the 
    // ToastNotification for the toast is retrieved
    XmlDocument^ toastXml = ToastNotificationManager::GetTemplateContent(ToastTemplateType::ToastText02);

    // Set both lines of text
    XmlNodeList^ nodeList = toastXml->GetElementsByTagName("text");
    nodeList->Item(0)->AppendChild(toastXml->CreateTextNode("Geolocation Sample"));

    if (1 == numEventsOfInterest)
    {
        nodeList->Item(1)->AppendChild(toastXml->CreateTextNode(eventName));
    }
    else
    {
        auto secondLine = "There are " + numEventsOfInterest + " new geofence events";
        nodeList->Item(1)->AppendChild(toastXml->CreateTextNode(secondLine));
    }

    // now create a xml node for the audio source
    IXmlNode^ toastNode = toastXml->SelectSingleNode("/toast");
    XmlElement^ audio = toastXml->CreateElement("audio");
    audio->SetAttribute("src", "ms-winsoundevent:Notification.SMS");

    ToastNotification^ toast = ref new ToastNotification(toastXml);
    ToastNotifier->Show(toast);
}

// Helper method to send notification text to the tile
void Scenario5::DoTile(int numEventsOfInterest, Platform::String^ eventName)
{
    // update tile
    TileUpdater^ tileUpdater = TileUpdateManager::CreateTileUpdaterForApplication();
    tileUpdater->EnableNotificationQueue(true);

    tileUpdater->Clear();

    Windows::Data::Xml::Dom::XmlDocument^ tileXml = TileUpdateManager::GetTemplateContent(TileTemplateType::TileSquare150x150Text02);

    Windows::Data::Xml::Dom::XmlNodeList^ tileNodeList = tileXml->GetElementsByTagName("text");
    tileNodeList->Item(0)->AppendChild(tileXml->CreateTextNode("Geolocation Sample"));

    if (1 == numEventsOfInterest)
    {
        tileNodeList->Item(1)->AppendChild(tileXml->CreateTextNode(eventName));
    }
    else
    {
        auto secondLine = "There are " + numEventsOfInterest + " new geofence events";
        tileNodeList->Item(1)->AppendChild(tileXml->CreateTextNode(secondLine));
    }

    TileNotification^ tile = ref new TileNotification(tileXml);
    tileUpdater->Update(tile);
}

// Helper method to update badge with number of events
void Scenario5::DoBadge(int numEventsOfInterest)
{
    BadgeUpdater^ badgeUpdater = BadgeUpdateManager::CreateBadgeUpdaterForApplication();

    Platform::String^ badgeXmlString = "<badge value='" + numEventsOfInterest + "'/>"; ;
    Windows::Data::Xml::Dom::XmlDocument^ badgeXML = ref new Windows::Data::Xml::Dom::XmlDocument();
    badgeXML->LoadXml(badgeXmlString);

    BadgeNotification^ badge = ref new BadgeNotification(badgeXML);
    badgeUpdater->Update(badge);
}
