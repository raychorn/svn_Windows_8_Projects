//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

// Custom templating function
var GeofenceBackgroundEventsItemTemplate = WinJS.Utilities.markSupportedForProcessing(function GeofenceBackgroundEventsItemTemplate(itemPromise) {
    return itemPromise.then(function (currentItem) {

        // Build ListView Item Container div
        var result = document.createElement("div");
        result.className = "GeofenceBackgroundEventsListViewItemStyle";

        // Build content body
        var body = document.createElement("div");

        // Display title
        if (currentItem.data) {
            var title = document.createElement("h4");
            title.innerText = currentItem.data;
            body.appendChild(title);
        }

        // put the body into the ListView Item
        result.appendChild(body);

        return result;
    });
});

(function () {
    "use strict";
    var Background = Windows.ApplicationModel.Background;

    var geofenceTask;
    var geolocator;
    var promise;
    var pageLoaded = false;
    var sampleBackgroundTaskName = "SampleGeofencingBackgroundTask";
    var sampleBackgroundTaskEntryPoint = "js\\geofencebackgroundtask.js";
    var dateResolutionToSeconds = 1000;  // conversion from 1 milli-second resolution to seconds
    var numEventsOfInterest = 0;

    var geofenceEventsData;
    var geofenceEventsListView;

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

    var page = WinJS.UI.Pages.define("/html/scenario5.html", {
        ready: function (element, options) {
            document.getElementById("registerBackgroundTaskButton").addEventListener("click", registerBackgroundTask, false);
            document.getElementById("unregisterBackgroundTaskButton").addEventListener("click", unregisterBackgroundTask, false);

            // Loop through all background tasks to see if our task is already registered
            var iter = Windows.ApplicationModel.Background.BackgroundTaskRegistration.allTasks.first();
            var hascur = iter.hasCurrent;
            while (hascur) {
                var cur = iter.current.value;
                if (cur.name === sampleBackgroundTaskName) {
                    geofenceTask = cur;
                    break;
                }
                hascur = iter.moveNext();
            }

            if (geofenceTask) {
                // Associate an event handler to the existing background task
                geofenceTask.addEventListener("completed", onCompleted);

                var backgroundAccessStatus = Background.BackgroundExecutionManager.getAccessStatus();
                switch (backgroundAccessStatus) {
                    case Background.BackgroundAccessStatus.unspecified:
                    case Background.BackgroundAccessStatus.denied:
                        WinJS.log && WinJS.log("This application must be added to the lock screen before the background task will run.", "sample", "status");
                        break;

                    default:
                        WinJS.log && WinJS.log("Background task is already registered. Waiting for next update...", "sample", "status");
                        break;
                }

                document.getElementById("registerBackgroundTaskButton").disabled = true;
                document.getElementById("unregisterBackgroundTaskButton").disabled = false;
            } else {
                document.getElementById("registerBackgroundTaskButton").disabled = false;
                document.getElementById("unregisterBackgroundTaskButton").disabled = true;
            }

            geolocator = new Windows.Devices.Geolocation.Geolocator();

            geofenceEventsData = new WinJS.Binding.List();

            geofenceEventsListView = element.querySelector('#GeofenceBackgroundEventsListView').winControl;
            geofenceEventsListView.forceLayout();

            var settings = Windows.Storage.ApplicationData.current.localSettings;
            if (settings.values.hasKey("GeofenceEvent")) {
                settings.values.remove("GeofenceEvent");
            }

            WinJS.UI.processAll().done(function () {
                geofenceEventsListView.itemDataSource = geofenceEventsData.dataSource;

                geofenceEventsListView.itemTemplate = GeofenceBackgroundEventsItemTemplate;
            });

            pageLoaded = true;
        },
        unload: function () {
            pageLoaded = false;
            
            if (promise) {
                promise.operation.cancel();
            }

            if (geofenceTask) {
                geofenceTask.removeEventListener("completed", onCompleted);
            }
        }
    });

    // Handle background task completion
    function onCompleted() {
        try {
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
            updateUIAfterPositionFix();
        } catch (ex) {
            // The background task had an error
            WinJS.log && WinJS.log(ex.toString(), "sample", "error");
        }
    }

    function registerBackgroundTask() {
        // Request lockscreen access
        Background.BackgroundExecutionManager.requestAccessAsync().done(
            function (backgroundAccessStatus) {
                var builder =  new Windows.ApplicationModel.Background.BackgroundTaskBuilder();

                // Register the background task
                builder.name = sampleBackgroundTaskName;
                builder.taskEntryPoint = sampleBackgroundTaskEntryPoint;
                builder.setTrigger(new Windows.ApplicationModel.Background.LocationTrigger(Windows.ApplicationModel.Background.LocationTriggerType.geofence));

                // If it is important that there is user presence and/or
                // internet connection when OnCompleted is called
                // the following could be called before calling Register()
                // var condition = new SystemCondition(SystemConditionType.userPresent | SystemConditionType.internetAvailable);
                // builder.addCondition(condition);

                geofenceTask = builder.register();

                geofenceTask.addEventListener("completed", onCompleted);

                document.getElementById("registerBackgroundTaskButton").disabled = true;
                document.getElementById("unregisterBackgroundTaskButton").disabled = false;

                switch (backgroundAccessStatus) {
                    case Background.BackgroundAccessStatus.unspecified:
                    case Background.BackgroundAccessStatus.denied:
                        WinJS.log && WinJS.log("This application must be added to the lock screen before the background task will run.", "sample", "status");
                        break;

                    default:
                        // Finish by getting an initial position. This will present the location consent
                        // dialog if it's the first attempt for this application to access location.
                        getGeopositionAsync();
                        break;
                }
            },
            function (e) {
                // Did you forget to do the background task declaration in the package manifest?
                WinJS.log && WinJS.log(e.toString(), "sample", "error");
            }
        );
    }

    function unregisterBackgroundTask() {
        // Remove the application from the lock screen
        Background.BackgroundExecutionManager.removeAccess();

        // Unregister the background task
        if (null !== geofenceTask) {
            geofenceTask.unregister(true);
            geofenceTask = null;
        }

        WinJS.log && WinJS.log("Background task unregistered", "sample", "status");

        document.getElementById("registerBackgroundTaskButton").disabled = false;
        document.getElementById("unregisterBackgroundTaskButton").disabled = true;
    }

    function getGeopositionAsync() {
        WinJS.log && WinJS.log("Checking permissions...", "sample", "status");

        promise = geolocator.getGeopositionAsync();
        promise.done(
            function (pos) {
                var coord = pos.coordinate;

                // got permissions so clear the status string
                WinJS.log && WinJS.log("", "sample", "status");
            },
            function (err) {
                if (pageLoaded) {
                    WinJS.log && WinJS.log(err.message, "sample", "error");
                }
            }
        );
    }

    function updateUIAfterPositionFix() {
        promise = geolocator.getGeopositionAsync();
        promise.done(
            function (pos) {
                updateUIWithPosition(pos);
            },
            function (err) {
                if (pageLoaded) {
                    WinJS.log && WinJS.log(err.message, "sample", "error");
                }
            }
        );
    }

    var BackgroundData = Object.freeze({
        year: 0,
        month: 1,
        day: 2,
        hour: 3,
        minute: 4,
        second: 5,
        period: 6,
        latitude: 7,
        longitude: 8,
        firsteventname: 9
    });

    function updateUIWithPosition(pos) { 
        // Update the UI with the completion status of the background task
        var settings = Windows.Storage.ApplicationData.current.localSettings;
        if (settings.values.hasKey("Status")) {
            WinJS.log && WinJS.log(settings.values["Status"], "sample", "status");
        }

        // pop a toast for each geofence event
        // and add to listview
        if (settings.values.hasKey("GeofenceEvent")) {
            var geofenceEvent = settings.values["GeofenceEvent"].toString();
            var eventArray = JSON.parse(geofenceEvent);
            var eventOfInterest = true;
            var calendar = new Windows.Globalization.Calendar();

            calendar.year = parseInt(eventArray[BackgroundData.year]);
            calendar.month = parseInt(eventArray[BackgroundData.month]);
            calendar.day = parseInt(eventArray[BackgroundData.day]);
            calendar.hour = parseInt(eventArray[BackgroundData.hour]);
            calendar.minute = parseInt(eventArray[BackgroundData.minute]);
            calendar.second = parseInt(eventArray[BackgroundData.second]);
            calendar.period = parseInt(eventArray[BackgroundData.period]);

            // NOTE TO DEVELOPER:
            // This event can be filtered out if the
            // geofence event location is stale.
            var eventDate = calendar.getDateTime();
            calendar.setToNow();
            var nowDate = calendar.getDateTime();
            var diffTimeSpan = nowDate.getTime() - eventDate.getTime();
            var deltaInSeconds = diffTimeSpan / dateResolutionToSeconds;

            // NOTE TO DEVELOPER:
            // If the time difference between the geofence event and now is too large
            // the eventOfInterest should be set to false.

            var eventItem = null;
            var arraySize = eventArray.length;
            numEventsOfInterest = (arraySize - BackgroundData.firsteventname)/2;

            if (eventOfInterest && 0 !== numEventsOfInterest) {
                var latitudeEvent = parseFloat(eventArray[BackgroundData.latitude]);
                var longitudeEvent = parseFloat(eventArray[BackgroundData.longitude]);

                // NOTE TO DEVELOPER:
                // This event can be filtered out if the
                // geofence event location is too far away.
                if ((latitudeEvent !== pos.coordinate.point.position.latitude) ||
                    (longitudeEvent !== pos.coordinate.point.position.longitude)) {
                    // NOTE TO DEVELOPER:
                    // Use an algorithm like Haversine to determine
                    // the distance between the current location (pos.coordinate)
                    // and the location of the geofence event (latitudeEvent/longitudeEvent).
                    // If too far apart set eventOfInterest to false to
                    // filter the event out.
                }

                if (eventOfInterest) {
                    for (var loop = BackgroundData.firsteventname; loop < arraySize;) {
                        var geofenceItemEvent = new String(eventArray[loop++]);

                        switch (eventArray[loop++]) {
                            case GeofenceReason.add:
                                geofenceItemEvent += " (Added)";
                                break;

                            case GeofenceReason.entered:
                                geofenceItemEvent += " (Entered)";
                                break;

                            case GeofenceReason.exited:
                                geofenceItemEvent += " (Exited)";
                                break;

                            case GeofenceReason.expired:
                                geofenceItemEvent += " (Removed/Expired)";
                                break;

                            case GeofenceReason.used:
                                geofenceItemEvent += " (Removed/Used)";
                                break;

                            default:
                                break;
                        }

                        if (0 !== geofenceItemEvent.length)
                        {
                            // now add event to listview
                            geofenceEventsData.unshift(geofenceItemEvent);
                            eventItem = geofenceItemEvent;
                        }
                        else
                        {
                            --numEventsOfInterest;
                        }
                    }
                }

                if (settings.values.hasKey("GeofenceEvent")) {
                    settings.values.remove("GeofenceEvent");
                }

                if (eventOfInterest && 0 !== numEventsOfInterest) {
                    doToast(numEventsOfInterest, eventItem.event);
                    doTile(numEventsOfInterest, eventItem.event);
                    doBadge(numEventsOfInterest);
                }
            }
        }
    }

    function doToast(numEventsOfInterestArg, eventName) {
        // pop a toast for each geofence event
        var toastNotifier = Windows.UI.Notifications.ToastNotificationManager.createToastNotifier();

        // Create a two line toast and add audio reminder

        // Here the xml that will be passed to the 
        // ToastNotification for the toast is retrieved
        // toastXml is an XmlDocument object
        var toastXml = Windows.UI.Notifications.ToastNotificationManager.getTemplateContent(Windows.UI.Notifications.ToastTemplateType.toastText02);

        // Set both lines of text
        // nodeList is an XmlNodeList object
        var nodeList = toastXml.getElementsByTagName("text");
        nodeList.item(0).appendChild(toastXml.createTextNode("Geolocation Sample"));

        if (1 === numEventsOfInterestArg) {
            nodeList.item(1).appendChild(toastXml.createTextNode(eventItem));
        } else {
            var secondLine = "There are " + numEventsOfInterestArg + " new geofence events";
            nodeList.item(1).appendChild(toastXml.createTextNode(secondLine));
        }

        // now create a xml node for the audio source
        // toastNode is an IXmlNode object
        var toastNode = toastXml.selectSingleNode("/toast");
        // audio is an XmlElement
        var audio = toastXml.createElement("audio");
        audio.setAttribute("src", "ms-winsoundevent:Notification.SMS");

        // toast is a ToastNotification object
        var toast = new Windows.UI.Notifications.ToastNotification(toastXml);
        toastNotifier.show(toast);
    }

    function doTile(numEventsOfInterestArg, eventName) {
        // update tile
        var tileUpdater = Windows.UI.Notifications.TileUpdateManager.createTileUpdaterForApplication();
        tileUpdater.enableNotificationQueue(true);

        tileUpdater.clear();

        var tileXml = Windows.UI.Notifications.TileUpdateManager.getTemplateContent(Windows.UI.Notifications.TileTemplateType.tileSquare150x150Text02);

        var tileNodeList = tileXml.getElementsByTagName("text");
        tileNodeList.item(0).appendChild(tileXml.createTextNode("Geolocation Sample"));

        if (1 === numEventsOfInterestArg) {
            tileNodeList.item(1).appendChild(tileXml.createTextNode(eventItem));
        } else {
            var secondLine = "There are " + numEventsOfInterestArg + " new geofence events";
            tileNodeList.item(1).appendChild(tileXml.createTextNode(secondLine));
        }

        var tile = new Windows.UI.Notifications.TileNotification(tileXml);
        tileUpdater.update(tile);
    }

    function doBadge(numEventsOfInterestArg) {
        var badgeUpdater = Windows.UI.Notifications.BadgeUpdateManager.createBadgeUpdaterForApplication();

        var badgeXmlString = "<badge value='" + numEventsOfInterestArg + "'/>";;
        var badgeXml = new Windows.Data.Xml.Dom.XmlDocument();
        badgeXml.loadXml(badgeXmlString);

        var badge = new Windows.UI.Notifications.BadgeNotification(badgeXml);
        badgeUpdater.update(badge);
    }
})();
