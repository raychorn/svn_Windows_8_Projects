//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

// Custom templating function
var RegisteredGeofencesItemTemplate = WinJS.Utilities.markSupportedForProcessing(function RegisteredGeofencesItemTemplate(itemPromise) {
    return itemPromise.then(function (currentItem) {

        // Build ListView Item Container div
        var result = document.createElement("div");
        result.className = "GeofenceListViewItemStyle";

        // Build content body
        var body = document.createElement("div");

        // Display title
        if (currentItem.data) {
            var title = document.createElement("h4");
            title.innerText = currentItem.data.id + " (" + currentItem.data.latitude + ", " + currentItem.data.longitude + ", " + currentItem.data.radius + ")";
            body.appendChild(title);
        }

        // put the body into the ListView Item
        result.appendChild(body);

        return result;
    });
});

// Custom templating function
var GeofenceEventsItemTemplate = WinJS.Utilities.markSupportedForProcessing(function GeofenceEventsItemTemplate(itemPromise) {
    return itemPromise.then(function (currentItem) {

        // Build ListView Item Container div
        var result = document.createElement("div");
        result.className = "GeofenceListViewItemStyle";

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
    var Enumeration = Windows.Devices.Enumeration;
    var DeviceAccessInformation = Enumeration.DeviceAccessInformation;
    var DeviceAccessStatus = Enumeration.DeviceAccessStatus;
    var geolocator = null;
    var accessInfo = null;
    var geofence = null;
    var geofenceItem = null;
    var message;
    var itemToRemove;
    var itemToRemoveId;
    var nameElement;
    var charCount;
    var latitude;
    var longitude;
    var radius;
    var geofenceSingleUse;
    var geofenceEnter;
    var geofenceExit;
    var geofenceRemove;
    var dwellTimeDays;
    var dwellTimeHours;
    var dwellTimeMinutes;
    var dwellTimeSeconds;
    var durationDays;
    var durationHours;
    var durationMinutes;
    var durationSeconds;
    var startTimeYear;
    var startTimeMonth;
    var startTimeDay;
    var startTimeHour;
    var startTimeMinute;
    var startTimeSecond;
    var promise;
    var geofenceStateChangedRegistered = false;
    var pageLoaded = false;
    var permissionsChecked = false;
    var inGetPositionAsync = false;
    var latitudeSet = false;
    var longitudeSet = false;
    var radiusSet = false;
    var nameSet = false;
    var secondsPerMinute = 60;
    var secondsPerHour = 60 * secondsPerMinute;
    var secondsPerDay = 24 * secondsPerHour;
    var millisecondsPerSecond = 1000;

    var maxDays = 999;
    var maxHours = 99;
    var maxMinutes = 99;
    var maxSeconds = 99;

    var GeofenceReason = Object.freeze({
        add: "Add",
        entered: "Entered",
        exited: "Exited",
        expired: "Removed/Expired",
        used: "Removed/Used",
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

    var registeredGeofenceData;
    var geofenceEventsData;
    var registeredGeofenceListView;
    var geofenceEventsListView;

    var GeofenceEventItem = WinJS.Class.define(
        function (id, reason) {
            this.id = id;
            this.reason = reason;
        }
    );

    var GeofenceItem = WinJS.Class.define(
        function (geofenceArg, reasonArg) {
            this.geofence = geofenceArg;
            this.id = geofenceArg.id;
            this.latitude = geofenceArg.geoshape.center.latitude;
            this.longitude = geofenceArg.geoshape.center.longitude;
            this.radius = geofenceArg.geoshape.radius;
            this.reason = reasonArg;
            this.dwellTime = geofenceArg.dwellTime;
            this.singleUse = geofenceArg.singleUse;
            this.mask = geofenceArg.monitoredStates;
            this.duration = geofenceArg.duration;
            this.startTime = geofenceArg.startTime;
        }, {
            geofenceStored: function () {
                return this.geofence;
            },
            reason: function () {
                return this.reason;
            },
            reasonString: function () {
                return this.reason.toString();
            },
            name: function () {
                return this.id;
            },
            latitude: function () {
                return this.latitude;
            },
            longitude: function () {
                return this.longitude;
            },
            radius: function () {
                return this.radius;
            },
            dwellTime: function () {
                return this.dwellTime;
            },
            singleUse: function () {
                return this.singleUse;
            },
            monitoredStates: function () {
                return this.mask;
            },
            duration: function () {
                return this.duration;
            },
            startTime: function () {
                return this.startTime;
            }
        }
    );

    var page = WinJS.UI.Pages.define("/html/scenario4.html", {
        ready: function (element, options) {
            document.getElementById("createGeofenceButton").addEventListener("click", createGeofence, false);
            document.getElementById("createGeofenceButton").disabled = true;
            document.getElementById("removeGeofenceItem").addEventListener("click", remove, false);
            document.getElementById("removeGeofenceItem").disabled = true;

            nameElement = document.getElementById("name");
            charCount = document.getElementById("charCount");
            latitude = document.getElementById("latitude");
            longitude = document.getElementById("longitude");
            radius = document.getElementById("radius");
            geofenceSingleUse = document.getElementById("geofenceSingleUse");
            geofenceEnter = document.getElementById("geofenceEnter");
            geofenceExit = document.getElementById("geofenceExit");
            geofenceRemove = document.getElementById("geofenceRemove");
            dwellTimeDays = document.getElementById("dwellTimeDays");
            dwellTimeHours = document.getElementById("dwellTimeHours");
            dwellTimeMinutes = document.getElementById("dwellTimeMinutes");
            dwellTimeSeconds = document.getElementById("dwellTimeSeconds");
            durationDays = document.getElementById("durationDays");
            durationHours = document.getElementById("durationHours");
            durationMinutes = document.getElementById("durationMinutes");
            durationSeconds = document.getElementById("durationSeconds");
            startTimeYear = document.getElementById("startTimeYear");
            startTimeMonth = document.getElementById("startTimeMonth");
            startTimeDay = document.getElementById("startTimeDay");
            startTimeHour = document.getElementById("startTimeHour");
            startTimeMinute = document.getElementById("startTimeMinute");
            startTimeSecond = document.getElementById("startTimeSecond");

            nameElement.addEventListener("input", nameInputHandler, false);
            latitude.addEventListener("input", latitudeInputHandler, false);
            longitude.addEventListener("input", longitudeInputHandler, false);
            radius.addEventListener("input", radiusInputHandler, false);
            dwellTimeDays.addEventListener("input", dwellTimeDaysInputHandler, false);
            dwellTimeHours.addEventListener("input", dwellTimeHoursInputHandler, false);
            dwellTimeMinutes.addEventListener("input", dwellTimeMinutesInputHandler, false);
            dwellTimeSeconds.addEventListener("input", dwellTimeSecondsInputHandler, false);
            durationDays.addEventListener("input", durationDaysInputHandler, false);
            durationHours.addEventListener("input", durationHoursInputHandler, false);
            durationMinutes.addEventListener("input", durationMinutesInputHandler, false);
            durationSeconds.addEventListener("input", durationSecondsInputHandler, false);
            startTimeYear.addEventListener("input", startTimeYearInputHandler, false);
            startTimeMonth.addEventListener("input", startTimeMonthInputHandler, false);
            startTimeDay.addEventListener("input", startTimeDayInputHandler, false);
            startTimeHour.addEventListener("input", startTimeHourInputHandler, false);
            startTimeMinute.addEventListener("input", startTimeMinuteInputHandler, false);
            startTimeSecond.addEventListener("input", startTimeSecondInputHandler, false);

            try {
                geolocator = Windows.Devices.Geolocation.Geolocator();

                geofenceStateChangedRegistered = true;

                registeredGeofenceData = new WinJS.Binding.List();
                geofenceEventsData = new WinJS.Binding.List();

                registeredGeofenceListView = element.querySelector('#RegisteredGeofenceListView').winControl;
                geofenceEventsListView = element.querySelector('#GeofenceEventsListView').winControl;
                registeredGeofenceListView.forceLayout();
                geofenceEventsListView.forceLayout();

                registeredGeofenceListView.addEventListener("selectionchanged", geofenceRegisteredListViewSelectionChangedHandler, false);

                accessInfo = DeviceAccessInformation.createFromDeviceClass(Enumeration.DeviceClass.location);
                accessInfo.addEventListener("accesschanged", onAccessChangedHandler);

                // register for geofence state change events
                Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.addEventListener("geofencestatechanged", onGeofenceStateChangedHandler);
                Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.addEventListener("statuschanged", onStatusChangedHandler);

                geofenceStateChangedRegistered = true;

                WinJS.UI.processAll().done(function () {
                    registeredGeofenceListView.itemDataSource = registeredGeofenceData.dataSource;
                    geofenceEventsListView.itemDataSource = geofenceEventsData.dataSource;

                    registeredGeofenceListView.itemTemplate = RegisteredGeofencesItemTemplate;
                    geofenceEventsListView.itemTemplate = GeofenceEventsItemTemplate;

                    fillRegisteredGeofenceListViewWithExistingGeofences();
                });
            } catch (ex) {
                if (ex.number === -2147024891) {
                    if (DeviceAccessStatus.deniedByUser === accessInfo.currentStatus) {
                        WinJS.log && WinJS.log("Location has been disabled by the user. Enable access through the settings charm.", "sample", "status");
                    } else if (DeviceAccessStatus.deniedBySystem === accessInfo.currentStatus) {
                        WinJS.log && WinJS.log("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", "sample", "status");
                    } else if (DeviceAccessStatus.unspecified === accessInfo.currentStatus) {
                        WinJS.log && WinJS.log("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", "sample", "status");
                    }
                } else {
                    WinJS.log && WinJS.log(ex.toString(), "sample", "error");
                }
            } finally {
                pageLoaded = true;
            }
        },
        unload: function () {
            Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.removeEventListener("geofencestatechanged", onGeofenceStateChangedHandler);
            Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.removeEventListener("statuschanged", onStatusChangedHandler);
            pageLoaded = false;
            if (inGetPositionAsync) {
                promise.operation.cancel();
            }
        }
    });

    function geofenceRegisteredListViewSelectionChangedHandler(eventInfo) {
        // update controls with the values from this geofence item
        // get selected item
        registeredGeofenceListView.selection.getItems().done(function completed(result) {
            if (0 !== result.length) {
                // enable the remove button
                document.getElementById("removeGeofenceItem").disabled = false;

                var item = result[0].data;

                refreshControlsFromGeofenceItem(item);

                document.getElementById("createGeofenceButton").disabled = !settingsAvailable();
            } else {
                // disable the remove button
                document.getElementById("removeGeofenceItem").disabled = true;
            }
        });
    }

    function fillRegisteredGeofenceListViewWithExistingGeofences() {
        Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.geofences.forEach(createGeofenceItemAndAddToListView);
    }

    function createGeofenceItemAndAddToListView(fence) {
        var item = new GeofenceItem(fence, GeofenceReason.add);

        registeredGeofenceData.unshift(item);
    }

    function getTimeStampedMessage(eventCalled) {
        var formatter = new Windows.Globalization.DateTimeFormatting.DateTimeFormatter("longtime");
        var calendar = new Windows.Globalization.Calendar();
        calendar.setToNow();

        message = eventCalled + " " + formatter.format(calendar.getDateTime());

        return message;
    }

    function onGeofenceStateChangedHandler(args) {
        try {
            args.target.readReports().forEach(processReport);
        } catch (ex) {
            WinJS.log && WinJS.log(ex.toString(), "sample", "error");
        }
    }

    function onStatusChangedHandler(args) {
        try {
            var eventDescription = getTimeStampedMessage("Geofence Status");
            var item = null;
            var geofenceStatus = args.target.status;

            if (Windows.Devices.Geolocation.Geofencing.GeofenceMonitorStatus.ready === geofenceStatus) {
                eventDescription += " (" + GeofenceReason.ready + ")";
            } else if (Windows.Devices.Geolocation.Geofencing.GeofenceMonitorStatus.initializing === geofenceStatus) {
                eventDescription += " (" + GeofenceReason.initializing + ")";
            } else if (Windows.Devices.Geolocation.Geofencing.GeofenceMonitorStatus.noData === geofenceStatus) {
                eventDescription += " (" + GeofenceReason.noData + ")";
            } else if (Windows.Devices.Geolocation.Geofencing.GeofenceMonitorStatus.disabled === geofenceStatus) {
                eventDescription += " (" + GeofenceReason.disabled + ")";
            } else if (Windows.Devices.Geolocation.Geofencing.GeofenceMonitorStatus.notInitialized === geofenceStatus) {
                eventDescription += " (" + GeofenceReason.notInitialized + ")";
            } else if (Windows.Devices.Geolocation.Geofencing.GeofenceMonitorStatus.notAvailable === geofenceStatus) {
                eventDescription += " (" + GeofenceReason.notAvailable + ")";
            }

            geofenceEventsData.unshift(eventDescription);
        } catch (ex) {
            WinJS.log && WinJS.log(ex.toString(), "sample", "error");
        }
    }

    function onAccessChangedHandler(args) {
        var eventDescription = getTimeStampedMessage("Device Access Status");
        var item = null;

        if (DeviceAccessStatus.deniedByUser === args.status) {
            eventDescription += " (" + GeofenceReason.deniedByUser + ")";

            geofenceEventsData.unshift(eventDescription);

            WinJS.log && WinJS.log("Location has been disabled by the user. Enable access through the settings charm.", "sample", "status");
        } else if (DeviceAccessStatus.deniedBySystem === args.status) {
            eventDescription += " (" + GeofenceReason.deniedBySystem + ")";

            geofenceEventsData.unshift(eventDescription);

            WinJS.log && WinJS.log("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", "sample", "status");
        } else if (DeviceAccessStatus.unspecified === args.status) {
            eventDescription += " (" + GeofenceReason.unspecified + ")";

            geofenceEventsData.unshift(eventDescription);

            WinJS.log && WinJS.log("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", "sample", "status");
        } else if (DeviceAccessStatus.allowed === args.status) {
            eventDescription += " (" + GeofenceReason.allowed + ")";

            geofenceEventsData.unshift(eventDescription);

            // clear status
            WinJS.log && WinJS.log("", "sample", "status");

            if (!geofenceStateChangedRegistered) {
                // register for state change events
                Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.addEventListener("geofencestatechanged", onGeofenceStateChangedHandler);
                Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.addEventListener("statuschanged", onStatusChangedHandler);

                geofenceStateChangedRegistered = true;

                WinJS.UI.processAll().done(function () {
                    registeredGeofenceListView.itemDataSource = registeredGeofenceData.dataSource;
                    geofenceEventsListView.itemDataSource = geofenceEventsData.dataSource;

                    registeredGeofenceListView.itemTemplate = RegisteredGeofencesItemTemplate;
                    geofenceEventsListView.itemTemplate = GeofenceEventsItemTemplate;

                    fillRegisteredGeofenceListViewWithExistingGeofences();
                });
            }
        } else {
            WinJS.log && WinJS.log("Unknown device access information status", "sample", "status");
        }
    }

    function processReport(report) {
        var state = report.newState;

        geofence = report.geofence;
        var eventDescription = getTimeStampedMessage(geofence.id);

        if (state === Windows.Devices.Geolocation.Geofencing.GeofenceState.removed) {
            var reason = report.RemovalReason;
            
            if (reason === Windows.Devices.Geolocation.Geofencing.GeofenceRemovalReason.expired) {
                eventDescription += " (" + GeofenceReason.expired + ")";
                geofenceEventsData.unshift(eventDescription);
            } else if (reason === Windows.Devices.Geolocation.Geofencing.GeofenceRemovalReason.used) {
                eventDescription += " (" + GeofenceReason.used + ")";
                geofenceEventsData.unshift(eventDescription);
            }

            // remove this item from the registered geofence collection
            itemToRemoveId = itemToRemove.id;

            // determine index at which itemToRemove is located
            var countInCollection = registeredGeofenceData.length;

            var foundInCollection = false;

            for (var loop = 0 ; loop < countInCollection ; loop++) {
                var itemInCollection = registeredGeofenceData.getAt(loop);

                if (itemToRemoveId === itemInCollection.id) {
                    registeredGeofenceData.splice(loop, 1);
                    foundInCollection = true;
                    break;
                }
            }

            if (!foundInCollection) {
                var msg = "Could not find GeofenceItem " + itemToRemoveId + " in Collection";;

                WinJS.log && WinJS.log(msg, "sample", "status");
            }
        } else if (state === Windows.Devices.Geolocation.Geofencing.GeofenceState.entered) {
            eventDescription += " (" + GeofenceReason.entered + ")";
            geofenceEventsData.unshift(eventDescription);
        } else if (state === Windows.Devices.Geolocation.Geofencing.GeofenceState.exited) {
            eventDescription += " (" + GeofenceReason.exited + ")";
            geofenceEventsData.unshift(eventDescription);
        }
    }

    function nameInputHandler() {
        // get number of characters
        if (nameElement.value) {
            var count = nameElement.value.length;
            charCount.innerText = count.toString() + " characters";
            if (0 !== count) {
                nameSet = true;
            } else {
                nameSet = false;
            }
        } else {
            charCount.innerText = "0 characters";
            nameSet = false;
        }

        document.getElementById("createGeofenceButton").disabled = !settingsAvailable();
    }

    function latitudeInputHandler() {
        if (textChangedHandlerDouble(false, "Latitude", latitude)) {
            latitudeSet = true;
        } else {
            latitudeSet = false;
        }

        document.getElementById("createGeofenceButton").disabled = !settingsAvailable();
    }

    function longitudeInputHandler() {
        if (textChangedHandlerDouble(false, "Longitude", longitude)) {
            longitudeSet = true;
        } else {
            longitudeSet = false;
        }

        document.getElementById("createGeofenceButton").disabled = !settingsAvailable();
    }

    function radiusInputHandler() {
        if (textChangedHandlerDouble(false, "Radius", radius)) {
            radiusSet = true;
        } else {
            radiusSet = false;
        }

        document.getElementById("createGeofenceButton").disabled = !settingsAvailable();
    }

    function dwellTimeDaysInputHandler() {
        textChangedHandlerInt(true, "Days", dwellTimeDays);
    }

    function dwellTimeHoursInputHandler() {
        textChangedHandlerInt(true, "Hours", dwellTimeHours);
    }

    function dwellTimeMinutesInputHandler() {
        textChangedHandlerInt(true, "Minutes", dwellTimeMinutes);
    }

    function dwellTimeSecondsInputHandler() {
        textChangedHandlerInt(true, "Seconds", dwellTimeSeconds);
    }

    function durationDaysInputHandler() {
        textChangedHandlerInt(true, "Days", durationDays);
    }

    function durationHoursInputHandler() {
        textChangedHandlerInt(true, "Hours", durationHours);
    }

    function durationMinutesInputHandler() {
        textChangedHandlerInt(true, "Minutes", durationMinutes);
    }

    function durationSecondsInputHandler() {
        textChangedHandlerInt(true, "Seconds", durationSeconds);
    }

    function startTimeYearInputHandler() {
        textChangedHandlerInt(true, "Year", startTimeYear);
    }

    function startTimeMonthInputHandler() {
        textChangedHandlerInt(true, "Monthe", startTimeMonth);
    }

    function startTimeDayInputHandler() {
        textChangedHandlerInt(true, "Day", startTimeDay);
    }

    function startTimeHourInputHandler() {
        textChangedHandlerInt(true, "Hour", startTimeHour);
    }

    function startTimeMinuteInputHandler() {
        textChangedHandlerInt(true, "Minute", startTimeMinute);
    }

    function startTimeSecondInputHandler() {
        textChangedHandlerInt(true, "Second", startTimeSecond);
    }

    function textChangedHandlerDouble(nullAllowed, elementName, e) {
        var valueSet = false;

        var decimalFormatter = new Windows.Globalization.NumberFormatting.DecimalFormatter();

        var stringValue = new String(e.value);

        var value = decimalFormatter.parseDouble(stringValue);

        if (!value) {
            var msg;
            // value is either empty or alphabetic
            if (0 === stringValue.length) {
                msg = elementName + " needs a value";

                WinJS.log && WinJS.log(msg, "sample", "status");
            } else {
                msg = elementName + " must be a number";

                WinJS.log && WinJS.log(msg, "sample", "status");
            }
        } else {
            valueSet = true;
        }

        if (valueSet) {
            // clear out status message
            WinJS.log && WinJS.log("", "sample", "status");
        }

        return valueSet;
    }

    function textChangedHandlerInt(nullAllowed, elementName, e) {
        var valueSet = false;

        var decimalFormatter = new Windows.Globalization.NumberFormatting.DecimalFormatter();

        var stringValue = new String(e.value);

        var value = decimalFormatter.parseInt(stringValue);

        if (!value) {
            var msg;
            // value is either empty or alphabetic
            if (0 === stringValue.length) {
                msg = elementName + " needs a value";

                WinJS.log && WinJS.log(msg, "sample", "status");
            } else {
                msg = elementName + " must be a number";

                WinJS.log && WinJS.log(msg, "sample", "status");
            }
        } else {
            valueSet = true;
        }

        if (valueSet) {
            // clear out status message
            WinJS.log && WinJS.log("", "sample", "status");
        }

        return valueSet;
    }

    function refreshControlsFromGeofenceItem(item) {
        if (item) {
            nameElement.value = item.id;
            latitude.value = item.latitude;
            longitude.value = item.longitude;
            radius.value = item.radius;

            geofenceSingleUse.checked = item.singleUse;

            var states = item.mask;
            var entered = Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.entered;
            var exited = Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.exited;
            var removed = Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.removed;
            geofenceEnter.checked = ((states & entered) === entered);
            geofenceExit.checked = ((states & exited) === exited);
            geofenceRemove.checked = ((states & removed) === removed);

            var totalSeconds = item.dwellTime / 1000;

            var dwellTimeDaysValue = totalSeconds / secondsPerDay;

            if (dwellTimeDaysValue >= 1) {
                if (maxDays < dwellTimeDaysValue) {
                    dwellTimeDaysValue = maxDays;
                }

                totalSeconds -= dwellTimeDaysValue * secondsPerDay;
            } else {
                dwellTimeDaysValue = 0;
            }

            var dwellTimeHoursValue = totalSeconds / secondsPerHour;

            if (dwellTimeHoursValue >= 1) {
                if (maxHours < dwellTimeHoursValue) {
                    dwellTimeHoursValue = maxHours;
                }

                totalSeconds -= dwellTimeHoursValue * secondsPerHour;
            } else {
                dwellTimeHoursValue = 0;
            }

            var dwellTimeMinutesValue = totalSeconds / secondsPerMinute;

            if (dwellTimeMinutesValue >= 1) {
                if (maxMinutes < dwellTimeMinutesValue) {
                    dwellTimeMinutesValue = maxMinutes;
                }

                totalSeconds -= dwellTimeMinutesValue * secondsPerMinute;
            } else {
                dwellTimeMinutesValue = 0;
            }

            var dwellTimeSecondsValue = totalSeconds;

            dwellTimeDays.value = (0 !== dwellTimeDaysValue) ? dwellTimeDaysValue : "";
            dwellTimeHours.value = (0 !== dwellTimeHoursValue) ? dwellTimeHoursValue : "";
            dwellTimeMinutes.value = (0 !== dwellTimeMinutesValue) ? dwellTimeMinutesValue : "";
            dwellTimeSeconds.value = (0 !== dwellTimeSecondsValue) ? dwellTimeSecondsValue : "";

            totalSeconds = item.duration / 1000;

            var durationDaysValue = totalSeconds / secondsPerDay;

            if (durationDaysValue >= 1) {
                if (maxDays < durationDaysValue) {
                    durationDaysValue = maxDays;
                }

                totalSeconds -= durationDaysValue * secondsPerDay;
            } else {
                durationDaysValue = 0;
            }

            var durationHoursValue = totalSeconds / secondsPerHour;

            if (durationHoursValue >= 1) {
                if (maxHours < durationHoursValue) {
                    durationHoursValue = maxHours;
                }

                totalSeconds -= durationHoursValue * secondsPerHour;
            } else {
                durationHoursValue = 0;
            }

            var durationMinutesValue = totalSeconds / secondsPerMinute;

            if (durationMinutesValue >= 1) {
                if (maxMinutes < durationMinutesValue) {
                    durationMinutesValue = maxMinutes;
                }

                totalSeconds -= durationMinutesValue * secondsPerMinute;
            } else {
                durationMinutesValue = 0;
            }

            var durationSecondsValue = totalSeconds;

            durationDays.value = (0 !== durationDaysValue) ? durationDaysValue : "";
            durationHours.value = (0 !== durationHoursValue) ? durationHoursValue : "";
            durationMinutes.value = (0 !== durationMinutesValue) ? durationMinutesValue : "";
            durationSeconds.value = (0 !== durationSecondsValue) ? durationSecondsValue : "";

            // if start date was not set item.startTime
            // will be -11644473600000
            // so just use values > 0
            if (0 < item.startTime) {
                var startDate = new Date(item.startTime);

                startTimeYear.value = (0 !== startDate.getYear()) ? startDate.getYear() : "";
                startTimeMonth.value = (0 !== startDate.getMonth()) ? startDate.getMonth() : "";
                startTimeDay.value = (0 !== startDate.getDay()) ? startDate.getDay() : "";
                startTimeHour.value = (0 !== startDate.getHours()) ? startDate.getHours() : "";
                startTimeMinute.value = (0 !== startDate.getMinutes()) ? startDate.getMinutes() : "";
                startTimeSecond.value = (0 !== startDate.getSeconds()) ? startDate.getSeconds() : "";
            } else {
                startTimeYear.value = "";
                startTimeMonth.value = "";
                startTimeDay.value = "";
                startTimeHour.value = "";
                startTimeMinute.value = "";
                startTimeSecond.value = "";
            }

            // Update flags used to enable Create Geofence button
            nameInputHandler();
            longitudeInputHandler();
            latitudeInputHandler();
            radiusInputHandler();
        }
    }

    // are settings available so a geofence can be created?
    function settingsAvailable() {
        var fSettingsAvailable = false;

        if (nameSet && latitudeSet && longitudeSet && radiusSet) {
            // also need to test if data is good
            fSettingsAvailable = true;
        }

        return fSettingsAvailable;
    }

    // add geofence to listview
    function addGeofenceToRegisteredGeofenceListView() {
        // call method that adds element to start of list
        // push adds element to end of list
        registeredGeofenceData.unshift(geofenceItem);
    }

    function remove() {
        try {
            registeredGeofenceListView.selection.getItems().done(function completed(result) {
                if (0 !== result.length) {
                    itemToRemove = result[0].data;
                    itemToRemoveId = itemToRemove.id;
                    var msg;

                    var geofenceToRemove = itemToRemove.geofenceStored();

                    var index = Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.geofences.indexOf(geofenceToRemove);

                    if (-1 !== index) {
                        Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.geofences.removeAt(index);
                    } else {
                        msg = "Could not find GeofenceItem " + itemToRemove.Name + " in GeofenceMonitor";

                        WinJS.log && WinJS.log(msg, "sample", "status");
                    }

                    // Remove this item from the registered geofence collection.
                    // WinJS.Binding.List does not have an indexOf method
                    // so will match based on names which are unique in
                    // each app.

                    // determine index at which itemToRemove is located
                    var countInCollection = registeredGeofenceData.length;

                    var foundInCollection = false;

                    for (var loop = 0 ; loop < countInCollection ; loop++) {
                        var itemInCollection = registeredGeofenceData.getAt(loop);

                        var itemInCollectionId = itemInCollection.id;

                        if (itemToRemoveId === itemInCollectionId) {
                            registeredGeofenceData.splice(loop, 1);
                            foundInCollection = true;
                            break;
                        }
                    }

                    if (!foundInCollection) {
                        msg = "Could not find GeofenceItem " + itemToRemoveId + " in Collection";

                        WinJS.log && WinJS.log(msg, "sample", "status");
                    }
                }
            });
        } catch (ex) {
            WinJS.log && WinJS.log(ex.toString(), "sample", "error");
        }
    }

    function createGeofence() {
        try {
            // This must be done here because there is no guarantee of 
            // getting the location consent from a geofence call.
            if (!permissionsChecked) {
                getGeoposition();
                permissionsChecked = true;
            }

            // get lat/long/radius, the fence name (fenceKey), 
            // and other properties from controls,
            // depending on data in controls for activation time
            // and duration the appropriate
            // constructor will be used.
            generateGeofence();

            // Add the geofence to the GeofenceMonitor's
            // collection of fences
            Windows.Devices.Geolocation.Geofencing.GeofenceMonitor.current.geofences.push(geofence);

            // add geofence to listview
            addGeofenceToRegisteredGeofenceListView();

        } catch (ex) {
            WinJS.log && WinJS.log(ex.toString(), "sample", "error");
        }
    }

    function generateGeofence() {
        try {
            geofence = null;
            geofenceItem = null;

            var fenceKey = nameElement.value;

            var decimalFormatter = new Windows.Globalization.NumberFormatting.DecimalFormatter();

            var position = {
                latitude: decimalFormatter.parseDouble(latitude.value),
                longitude: decimalFormatter.parseDouble(longitude.value),
                altitude: 0
            };
            var radiusValue = decimalFormatter.parseDouble(radius.value);

            // the geofence is a circular region
            var geocircle = new Windows.Devices.Geolocation.Geocircle(position, radiusValue);

            var singleUse = false;

            if (geofenceSingleUse.checked) {
                singleUse = true;
            }

            // want to listen for enter geofence, exit geofence and remove geofence events
            // you can select a subset of these event states
            var mask = 0;

            if (geofenceEnter.checked) {
                mask = mask | Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.entered;
            }
            if (geofenceExit.checked) {
                mask = mask | Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.exited;
            }
            if (geofenceRemove.checked) {
                mask = mask | Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.removed;
            }

            // setting up how long you need to be in geofence for enter event to fire
            var dwellTimeDaysValue = 0;
            var dwellTimeHoursValue = 0;
            var dwellTimeMinutesValue = 0;
            var dwellTimeSecondsValue = 0;

            var useDwellTime = false;

            if (textChangedHandlerInt(true, null, dwellTimeDays)) {
                dwellTimeDaysValue = decimalFormatter.parseInt(dwellTimeDays.value);
            }
            if (textChangedHandlerInt(true, null, dwellTimeHours)) {
                dwellTimeHoursValue = decimalFormatter.parseInt(dwellTimeHours.value);
            }
            if (textChangedHandlerInt(true, null, dwellTimeMinutes)) {
                dwellTimeMinutesValue = decimalFormatter.parseInt(dwellTimeMinutes.value);
            }
            if (textChangedHandlerInt(true, null, dwellTimeSeconds)) {
                dwellTimeSecondsValue = decimalFormatter.parseInt(dwellTimeSeconds.value);
            }

            if (0 !== dwellTimeDaysValue || 0 !== dwellTimeHoursValue || 0 !== dwellTimeMinutesValue || 0 !== dwellTimeSecondsValue) {
                useDwellTime = true;
            }

            // setting up how long the geofence should be active
            var durationDaysValue = 0;
            var durationHoursValue = 0;
            var durationMinutesValue = 0;
            var durationSecondsValue = 0;

            var useDuration = false;

            // use duration if at least one textbox has text
            if (textChangedHandlerInt(true, null, durationDays)) {
                durationDaysValue = decimalFormatter.parseInt(durationDays.value);
            }
            if (textChangedHandlerInt(true, null, durationHours)) {
                durationHoursValue = decimalFormatter.parseInt(durationHours.value);
            }
            if (textChangedHandlerInt(true, null, durationMinutes)) {
                durationMinutesValue = decimalFormatter.parseInt(durationMinutes.value);
            }
            if (textChangedHandlerInt(true, null, durationSeconds)) {
                durationSecondsValue = decimalFormatter.parseInt(durationSeconds.value);
            }

            if (0 !== durationDaysValue || 0 !== durationHoursValue || 0 !== durationMinutesValue || 0 !== durationSecondsValue) {
                useDuration = true;
            }

            // setting up the start time of the geofence
            var startTimeYearValue = 0;
            var startTimeMonthValue = 0;
            var startTimeDayValue = 0;
            var startTimeHourValue = 0;
            var startTimeMinuteValue = 0;
            var startTimeSecondValue = 0;

            var useStartTime = false;

            // use duration if at least one textbox has text
            if (textChangedHandlerInt(true, null, startTimeYear)) {
                startTimeYearValue = decimalFormatter.parseInt(startTimeYear.value);
            }
            if (textChangedHandlerInt(true, null, startTimeMonth)) {
                startTimeMonthValue = decimalFormatter.parseInt(startTimeMonth.value);
            }
            if (textChangedHandlerInt(true, null, startTimeDay)) {
                startTimeDayValue = decimalFormatter.parseInt(startTimeDay.value);
            }
            if (textChangedHandlerInt(true, null, startTimeHour)) {
                startTimeHourValue = decimalFormatter.parseInt(startTimeHour.value);
            }
            if (textChangedHandlerInt(true, null, startTimeMinute)) {
                startTimeMinuteValue = decimalFormatter.parseInt(startTimeMinute.value);
            }
            if (textChangedHandlerInt(true, null, startTimeSecond)) {
                startTimeSecondValue = decimalFormatter.parseInt(startTimeSecond.value);
            }

            if (0 !== startTimeYearValue || 0 !== startTimeMonthValue || 0 !== startTimeDayValue || 0 !== startTimeHourValue || 0 !== startTimeMinuteValue || 0 !== startTimeSecondValue) {
                useStartTime = true;
            }

            var totalSeconds;
            var dwellTime;

            if (!useStartTime && !useDuration) {
                if (useDwellTime) {
                    // count number of seconds
                    totalSeconds = dwellTimeSecondsValue + dwellTimeMinutesValue*secondsPerMinute + dwellTimeHoursValue*secondsPerHour + dwellTimeDaysValue*secondsPerDay;

                    totalSeconds *= millisecondsPerSecond;

                    dwellTime = new Number(totalSeconds);

                    // since mask cannot be zero, use the default values used by the geofence
                    if (0 === mask) {
                        mask = Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.entered | Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.exited;
                    }

                    geofence = new Windows.Devices.Geolocation.Geofencing.Geofence(fenceKey, geocircle, mask, singleUse, dwellTime);
                } else {
                    if (0 !== mask) {
                        geofence = new Windows.Devices.Geolocation.Geofencing.Geofence(fenceKey, geocircle, mask, singleUse);
                    } else {
                        geofence = new Windows.Devices.Geolocation.Geofencing.Geofence(fenceKey, geocircle);
                    }
                }
            } else {
                // count number of seconds
                totalSeconds = dwellTimeSecondsValue + dwellTimeMinutesValue * secondsPerMinute + dwellTimeHoursValue * secondsPerHour + dwellTimeDaysValue * secondsPerDay;

                totalSeconds *= millisecondsPerSecond;

                dwellTime = new Number(totalSeconds);

                // count number of seconds
                totalSeconds = durationSecondsValue + durationMinutesValue*secondsPerMinute + durationHoursValue*secondsPerHour + durationDaysValue*secondsPerDay;

                totalSeconds *= millisecondsPerSecond;

                var duration = new Number(totalSeconds);

                var startTime = new Date();

                startTime.setYear(startTimeYearValue);
                startTime.setMonth(startTimeMonthValue);
                startTime.setDate(startTimeDayValue);
                startTime.setHours(startTimeHourValue);
                startTime.setMinutes(startTimeMinuteValue);
                startTime.setSeconds(startTimeSecondValue);

                // since mask cannot be zero, use the default values used by the geofence
                if (0 === mask) {
                    mask = Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.entered | Windows.Devices.Geolocation.Geofencing.MonitoredGeofenceStates.exited;
                }

                geofence = new Windows.Devices.Geolocation.Geofencing.Geofence(fenceKey, geocircle, mask, singleUse, dwellTime, startTime, duration);
            }

            if (geofence) {
                geofenceItem = new GeofenceItem(geofence, GeofenceReason.add);
            }
        } catch (ex) {
            WinJS.log && WinJS.log(ex.toString(), "sample", "error");
        }
    }

    function getGeoposition() {
        WinJS.log && WinJS.log("Checking permissions...", "sample", "status");

        inGetPositionAsync = true;

        promise = geolocator.getGeopositionAsync();
        promise.done(
            function (pos) {
                var coord = pos.coordinate;

                // clear status
                WinJS.log && WinJS.log("", "sample", "status");
            },
            function (err) {
                if (pageLoaded) {
                    WinJS.log && WinJS.log(err.message, "sample", "error");
                }
            }
        );

        inGetPositionAsync = false;
    }
})();
