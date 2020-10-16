//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// LocationBackgroundTask.cpp
#include "pch.h"
#include "LocationBackgroundTask.h"

using namespace Concurrency;
using namespace Platform;
using namespace BackgroundTask;
using namespace Windows::ApplicationModel::Background;
using namespace Windows::Data::Json;
using namespace Windows::Devices::Geolocation;
using namespace Windows::Devices::Geolocation::Geofencing;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Globalization::DateTimeFormatting;
using namespace Windows::Storage;

LocationBackgroundTask::LocationBackgroundTask()
{
}

LocationBackgroundTask::~LocationBackgroundTask()
{
}

void LocationBackgroundTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    // Get the deferral object from the task instance
    Platform::Agile<BackgroundTaskDeferral> deferral(taskInstance->GetDeferral());

    // Associate a cancellation handler with the background task
    taskInstance->Canceled += ref new BackgroundTaskCanceledEventHandler(this, &LocationBackgroundTask::OnCanceled);

    Geolocator^ geolocator = ref new Geolocator();

    task<Geoposition^> geopositionTask(geolocator->GetGeopositionAsync(), geopositionTaskTokenSource.get_token());
    geopositionTask.then([this, deferral, geolocator](task<Geoposition^> getPosTask)
    {
        DateTimeFormatter^ dateFormatter = ref new DateTimeFormatter("longtime");
        auto settings = ApplicationData::Current->LocalSettings;

        try
        {
            // Get will throw an exception if the task was canceled or failed with an error
            Geoposition^ pos = getPosTask.get();

            // Write to LocalSettings to indicate that this background task ran
            settings->Values->Insert("Status", "Time: " + dateFormatter->Format(pos->Coordinate->Timestamp));
            settings->Values->Insert("Latitude", pos->Coordinate->Point->Position.Latitude.ToString());
            settings->Values->Insert("Longitude", pos->Coordinate->Point->Position.Longitude.ToString());
            settings->Values->Insert("Accuracy", pos->Coordinate->Accuracy.ToString());
        }
        catch (Platform::AccessDeniedException^)
        {
            // Write to LocalSettings to indicate that this background task ran
            settings->Values->Insert("Status", "Disabled");
            settings->Values->Insert("Latitude", "No data");
            settings->Values->Insert("Longitude", "No data");
            settings->Values->Insert("Accuracy", "No data");
        }
        catch (task_canceled&)
        {
        }
        catch (Exception^ ex)
        {
            settings->Values->Insert("Latitude", "No data");
            settings->Values->Insert("Longitude", "No data");
            settings->Values->Insert("Accuracy", "No data");

            // If there are no location sensors GetGeopositionAsync()
            // will timeout -- that is acceptable.

            if (ex->HResult == HRESULT_FROM_WIN32(WAIT_TIMEOUT))
            {
                settings->Values->Insert("Status", "Operation accessing location sensors timed out. Possibly there are no location sensors.");
            }
            else
            {
                settings->Values->Insert("Status", ex->ToString());
            }
        }

        // Indicate that the background task has completed
        deferral->Complete();
    });
}

// Handles background task cancellation
void LocationBackgroundTask::OnCanceled(IBackgroundTaskInstance^ taskInstance, BackgroundTaskCancellationReason reason)
{
    // Cancel the async operation
    geopositionTaskTokenSource.cancel();
}

GeofenceBackgroundTask::GeofenceBackgroundTask()
{
}

GeofenceBackgroundTask::~GeofenceBackgroundTask()
{
}

void GeofenceBackgroundTask::Invoke(Windows::Foundation::IAsyncAction^ asyncInfo, Windows::Foundation::AsyncStatus asyncStatus)
{
}

void GeofenceBackgroundTask::Run(IBackgroundTaskInstance^ taskInstance)
{
    // Get the deferral object from the task instance
    Platform::Agile<BackgroundTaskDeferral> deferral(taskInstance->GetDeferral());

    // Associate a cancellation handler with the background task
    taskInstance->Canceled += ref new BackgroundTaskCanceledEventHandler(this, &GeofenceBackgroundTask::OnCanceled);

    auto taskReports = GetGeofenceStateChangedReportsAsync();

    // wait for work to be done
    taskReports.then([this, deferral] () {
        // Indicate that the background task has completed
        deferral->Complete();
    });
}

// Handles background task cancellation
void GeofenceBackgroundTask::OnCanceled(IBackgroundTaskInstance^ taskInstance, BackgroundTaskCancellationReason reason)
{
    // Cancel the async operation
    geopositionTaskTokenSource.cancel();
}

void GeofenceBackgroundTask::WipeGeofenceDataFromAppdata()
{
    auto settings = ApplicationData::Current->LocalSettings->Values;
    settings->Remove("GeofenceEvent");
}

void GeofenceBackgroundTask::WriteStatusToAppdata(Platform::String^ status)
{
    auto settings = ApplicationData::Current->LocalSettings->Values;
    settings->Insert("Status", status->ToString());
}

// Stores geofence events into local settings
task<void> GeofenceBackgroundTask::GetGeofenceStateChangedReportsAsync()
{
    return create_task([this]
    {
        JsonArray^ jsonArray = ref new JsonArray();

        GeofenceMonitor^ monitor = GeofenceMonitor::Current;

        Geoposition^ pos = monitor->LastKnownGeoposition;

        Windows::Globalization::Calendar^ calendar = ref new Windows::Globalization::Calendar();
        calendar->SetDateTime(pos->Coordinate->Timestamp);
        
        jsonArray->Append(JsonValue::CreateNumberValue(calendar->Year));
        jsonArray->Append(JsonValue::CreateNumberValue(calendar->Month));
        jsonArray->Append(JsonValue::CreateNumberValue(calendar->Day));
        jsonArray->Append(JsonValue::CreateNumberValue(calendar->Hour));
        jsonArray->Append(JsonValue::CreateNumberValue(calendar->Minute));
        jsonArray->Append(JsonValue::CreateNumberValue(calendar->Second));
        jsonArray->Append(JsonValue::CreateNumberValue(calendar->Period)); // 1:AM or 2:PM

        jsonArray->Append(JsonValue::CreateNumberValue(pos->Coordinate->Point->Position.Latitude));
        jsonArray->Append(JsonValue::CreateNumberValue(pos->Coordinate->Point->Position.Longitude));
        
        // Retreive a vector of geofence state changed reports
        auto reports = monitor->ReadReports();

        unsigned int count = reports->Size;

        for (unsigned int loop = 0; loop < count; loop++)
        {
            auto report = reports->GetAt(loop);

            GeofenceState state = report->NewState;

            jsonArray->Append(JsonValue::CreateStringValue(report->Geofence->Id));

            if (state == GeofenceState::Removed)
            {
                GeofenceRemovalReason reason = report->RemovalReason;

                if (reason == GeofenceRemovalReason::Expired)
                {
                    jsonArray->Append(JsonValue::CreateNumberValue(safe_cast<double>(GeofenceReason::Expired)));
                }
                else if (reason == GeofenceRemovalReason::Used)
                {
                    jsonArray->Append(JsonValue::CreateNumberValue(safe_cast<double>(GeofenceReason::Used)));
                }
            }
            else if (state == GeofenceState::Entered)
            {
                jsonArray->Append(JsonValue::CreateNumberValue(safe_cast<double>(GeofenceReason::Entered)));
            }
            else if (state == GeofenceState::Exited)
            {
                jsonArray->Append(JsonValue::CreateNumberValue(safe_cast<double>(GeofenceReason::Exited)));
            }
        }

        String^ jsonString = jsonArray->Stringify();

        auto settings = ApplicationData::Current->LocalSettings->Values;
        settings->Insert("GeofenceEvent", jsonString);
    });
}
