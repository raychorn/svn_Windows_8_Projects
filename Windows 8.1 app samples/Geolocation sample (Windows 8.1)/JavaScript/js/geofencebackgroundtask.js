//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

// A JavaScript background task runs a specified JavaScript file.
(function () {
    "use strict";
    var cancel = false;

    var GeofenceReason = Object.freeze({
        add: "Add",
        entered: "Entered",
        exited: "Exited",
        expired: "Expired",
        used: "Used",
        ready: "Ready",
        initializing: "Initializing",
        noData: "NoData",
        disabled: "Disabled",
        notInitialized: "NotInitialized",
        notAvailable: "NotAvailable",
        deniedByUser: "DeniedByUser",
        deniedBySystem: "DeniedBySystem",
        unspecified: "Unspecified",
        allowed: "Allowed"
    });

    // Get the background task instance's activation parameters
    var backgroundTaskInstance = Windows.UI.WebUI.WebUIBackgroundTaskInstance.current;

    // Associate a cancellation handler with the background task.
    function onCanceled(cancelSender, cancelReason) {
        cancel = true;
    }
    backgroundTaskInstance.addEventListener("canceled", onCanceled);

    var monitor = GeofenceMonitor.current;

    var pos = monitor.lastKnownGeoposition;

    var calendar = new Windows.Globalization.Calendar();
    calendar.setDateTime(pos.coordinate.Timestamp);

    var eventArray = new Array();

    var loop = 0;

    eventArray[loop++] = calendar.year;
    eventArray[loop++] = calendar.month;
    eventArray[loop++] = calendar.day;
    eventArray[loop++] = calendar.hour;
    eventArray[loop++] = calendar.minute;
    eventArray[loop++] = calendar.second;
    eventArray[loop++] = calendar.period; // 1:AM or 2:PM

    eventArray[loop++] = pos.coordinate.point.position.latitude;
    eventArray[loop++] = pos.coordinate.point.position.longitude;

    for (var report in Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.readReports()) {
        if (!cancel) {
            var state = report.newState;

            eventArray[loop++] = report.geofence.id;

            if (state === Windows.Devices.Geolocation.Geofencing.GeofenceState.removed) {
                var reason = report.removalReason;

                if (reason === Windows.Devices.Geolocation.Geofencing.GeofenceRemovalReason.expired) {
                    eventArray[loop++] = GeofenceReason.expired;
                } else if (reason === Windows.Devices.Geolocation.Geofencing.GeofenceRemovalReason.used) {
                    eventArray[loop++] = GeofenceReason.used;
                }
            } else if (state === Windows.Devices.Geolocation.Geofencing.GeofenceState.entered) {
                eventArray[loop++] = GeofenceReason.entered;
            } else if (state === Windows.Devices.Geolocation.Geofencing.GeofenceState.exited) {
                eventArray[loop++] = GeofenceReason.exited;
            }
        }
    }

    if (!cancel) {
        var settings = Windows.Storage.ApplicationData.current.localSettings;

        settings.values["GeofenceEvent"] = JSON.stringify(eventArray);
    }

    backgroundTaskInstance.succeeded = !cancel;

    // A JavaScript background task must call close when it is done
    close();
})();
