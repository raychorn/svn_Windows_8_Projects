﻿//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";

    /// <summary>
    /// The purpose of this class is to demonstrate what to do to a UsbDevice when a specific app event
    /// is raised (app suspension and resume) or when the device is disconnected. In addition to handling
    /// the UsbDevice, the app's state should also be saved upon app suspension (will not be demonstrated here).
    /// 
    /// This class will also demonstrate how to handle device watcher events.
    /// 
    /// For simplicity, this class will only allow at most one device to be connected at any given time. In order
    /// to make this class support multiple devices, make this class a non-singleton and create multiple instances
    /// of this class; each instance should watch one connected device.
    /// </summary>
    var EventHandlerForDeviceClass = WinJS.Class.define(
        /// <summary>
        /// If this event handler will be running in a background task, app events will not be registered for because they are of
        /// no use to the background task.
        /// </summary>
        /// <param name="isBackgroundTask">Whether or not the event handler will be running as a background task</param>
        function (isBackgroundTask) {
            this._isBackgroundTask = isBackgroundTask;
        }, {
        _appSuspendCallback: null,
        _deviceWatcher: null,
        _deviceSelector: null,
        _deviceInformation: null,
        _device: null,
        _watcherSuspended: false,
        _watcherStarted: false,
        _isBackgroundTask: false,
        _isEnabledAutoReconnect: true,
        onAppSuspendCallback: {
            get: function () {
                return this._appSuspendCallback;
            },
            set: function (newSuspensionHandler) {
                this._appSuspendCallback = newSuspensionHandler;
            }
        },
        isDeviceConnected: {
            get: function () {
                return this._device !== null;
            }
        },
        device: {
            get: function () {
                return this._device;
            }
        },
        /// <summary>
        /// This DeviceInformation represents which device is connected or which device will be reconnected when
        /// the device is plugged in again (if IsEnabledAutoReconnect is true);.
        /// </summary>
        deviceInformation: {
            get: function () {
                return this._deviceInformation;
            }
        },
        /// <summary>
        /// True if EventHandlerForDevice will attempt to reconnect to the device once it is plugged into the computer again
        /// </summary>
        isEnabledAutoReconnect: {
            get: function () {
                return this._isEnabledAutoReconnect;
            },
            set: function (value) {
                this._isEnabledAutoReconnect = value;
            }
        },
        /// <summary>
        /// DeviceSelector AQS used to find this device
        /// </summary>
        deviceSelector: {
            get: function() {
                return this._deviceSelector;
            }
        },
        /// <summary>
        /// This method opens the device using the WinRT Usb API. After the device is opened, we will save the device
        /// so that it can be used across scenarios.
        ///
        /// This method is used to reopen the device after the device reconnects to the computer and when the app resumes.
        /// </summary>
        /// <param name="deviceInfo">Device information of the device to be opened</param>
        /// <param name="deviceSelector">The AQS used to find this device</param>
        /// <returns>A promise with value of True if the device was successfully opened, false if the device could not be opened for well known reasons.
        /// An exception may be thrown if the device could not be opened for extraordinary reasons.</returns>
        openDeviceAsync: function (deviceInfo, deviceSelector) {
            this._deviceInformation = deviceInfo;
            this._deviceSelector = deviceSelector;

            return Windows.Devices.Usb.UsbDevice.fromIdAsync(deviceInfo.id).then(function (usbDevice) {
                var successfullyOpenedDevice = false;

                // Device could have been blocked by user or the device has already been opened by another app.
                if (usbDevice) {
                    WinJS.log && WinJS.log("Device " + deviceInfo.id + " opened", "sample", "status");

                    successfullyOpenedDevice = true;

                    EventHandlerForDeviceClass.current._device = usbDevice;

                    // Create and register device watcher events for the device to be opened unless we're reopening the device
                    if (!EventHandlerForDeviceClass.current._deviceWatcher) {
                        EventHandlerForDeviceClass.current._deviceWatcher = Windows.Devices.Enumeration.DeviceInformation.createWatcher(deviceSelector, null);
                    }

                    if (!EventHandlerForDeviceClass.current._watcherStarted) {
                        // Background tasks are not part of the app, so app events will not have an affect on the device
                        if (!EventHandlerForDeviceClass.current._isBackgroundTask) {
                            EventHandlerForDeviceClass.current._registerForAppEvents();
                        }

                        EventHandlerForDeviceClass.current._registerForDeviceWatcherEvents();

                        // Start the device watcher after we made sure that the device is opened.
                        EventHandlerForDeviceClass.current._startDeviceWatcher();
                    }
                } else {
                    successfullyOpenedDevice = false;

                    var deviceAccessStatus = Windows.Devices.Enumeration.DeviceAccessInformation.createFromId(deviceInfo.id).currentStatus;

                    if (deviceAccessStatus === Windows.Devices.Enumeration.DeviceAccessStatus.deniedByUser) {
                        WinJS.log && WinJS.log("Access to the device was blocked by the user : " + deviceInfo.id, "sample", "error");
                    } else if (deviceAccessStatus === Windows.Devices.Enumeration.DeviceAccessStatus.deniedBySystem) {
                        // This status is most likely caused by app permissions (did not declare the device in the app's package.appxmanifest)
                        // This status does not cover the case where the device is already opened by another app.
                        WinJS.log && WinJS.log("Access to the device was blocked by the system : " + deviceInfo.id, "sample", "error");
                    } else {
                        // Most likely the device is opened by another app, but cannot be sure
                        WinJS.log && WinJS.log("Unknown error, possibly opened by another app : " + deviceInfo.id, "sample", "error");
                    }
                }

                return successfullyOpenedDevice;
            });
        },
        /// <summary>
        /// Closes the device and stops the device watcher.
        /// </summary>
        closeDevice: function () {
            if (this.isDeviceConnected) {
                this._closeCurrentlyConnectedDevice();
            }

            if (this._deviceWatcher) {
                if (this._watcherStarted) {
                    this._stopDeviceWatchers();
                }

                this._deviceWatcher = null;
            }

            this._unregisterFromAppEvents();

            this._appSuspendCallback = null;
        },
        /// <summary>
        /// This method demonstrates how to close the device properly using the WinRT Usb API.
        ///
        /// When the UsbDevice is closing, it will cancel all IO operations that are still pending (not complete).
        /// The close will not wait for any IO completion callbacks to be called, so the close call may complete before any of
        /// the IO completion callbacks are called.
        /// The pending IO operations will still call their respective completion callbacks with either a task 
        /// cancelled error or the operation completed.
        /// </summary>
        _closeCurrentlyConnectedDevice: function () {
            if (this._device) {
                this._device.close();

                this._device = null;

                WinJS.log && WinJS.log(this._deviceInformation.id + " is closed", "sample", "status");
            }
        },
        /// <summary>
        /// Register for app suspension/resume events. See the comments
        /// for the event handlers for more information on what is being done to the device.
        ///
        /// We will also register for when the app exists so that we may close the device handle.
        /// </summary>
        _registerForAppEvents: function () {
            // This event is raised when the app is exited and when the app is suspended
            Windows.UI.WebUI.WebUIApplication.addEventListener("suspending", this._onAppSuspension);

            Windows.UI.WebUI.WebUIApplication.addEventListener("resuming", this._onAppResume);
        },
        _unregisterFromAppEvents: function () {
            // This event is raised when the app is exited and when the app is suspended
            Windows.UI.WebUI.WebUIApplication.removeEventListener("suspending", this._onAppSuspension);

            Windows.UI.WebUI.WebUIApplication.removeEventListener("resuming", this._onAppResume);
        },
        /// <summary>
        /// Register for Added, Removed, and EnumerationCompleted events.
        /// </summary>
        _registerForDeviceWatcherEvents: function () {
            this._deviceWatcher.addEventListener("added", this._onDeviceAdded, false);
            this._deviceWatcher.addEventListener("removed", this._onDeviceRemoved, false);
        },
        _startDeviceWatcher: function () {
            this._watcherStarted = true;

            if (this._deviceWatcher.status !== Windows.Devices.Enumeration.DeviceWatcherStatus.started
                && this._deviceWatcher.status !== Windows.Devices.Enumeration.DeviceWatcherStatus.enumerationCompleted) {
                this._deviceWatcher.start();
            }
        },
        _stopDeviceWatchers: function () {
            if (this._deviceWatcher.status === Windows.Devices.Enumeration.DeviceWatcherStatus.started
                || this._deviceWatcher.status === Windows.Devices.Enumeration.DeviceWatcherStatus.enumerationCompleted) {
                this._deviceWatcher.stop();
            }

            this._watcherStarted = false;
        },
        /// <summary>
        /// If a UsbDevice object has been instantiated (a handle to the device is opened), we must close it before the app 
        /// goes into suspension because the API automatically closes it for us if we don't. When resuming, the API will
        /// not reopen the device automatically, so we need to explicitly open the device in the app (Scenario1_DeviceConnect).
        ///
        /// Since we have to reopen the device ourselves when the app resumes, it is good practice to explicitly call the close
        /// in the app as well (For every open there is a close).
        /// 
        /// We must stop the DeviceWatcher because it will continue to raise events even if
        /// the app is in suspension, which is not desired (drains battery). We resume the device watcher once the app resumes again.
        /// </summary>
        /// <param name="suspendingEventArgs"></param>
        _onAppSuspension: function (suspendingEventArgs) {
            if (EventHandlerForDeviceClass.current._watcherStarted) {
                EventHandlerForDeviceClass.current._watcherSuspended = true;

                EventHandlerForDeviceClass.current._stopDeviceWatchers();

            } else {
                EventHandlerForDeviceClass.current._watcherSuspended = false;
            }

            // Forward suspend event to registered callback function
            if (EventHandlerForDeviceClass.current._appSuspendCallback) {
                EventHandlerForDeviceClass.current._appSuspendCallback(suspendingEventArgs);
            }

            EventHandlerForDeviceClass.current._closeCurrentlyConnectedDevice();
        },
        /// <summary>
        /// When resume into the application, we should reopen a handle to the Usb device again. This will automatically
        /// happen when we start the device watcher again; the device will be re-enumerated and we will attempt to reopen it
        /// if IsEnabledAutoReconnect property is enabled.
        /// 
        /// See OnAppSuspension for why we are starting the device watcher again
        /// </summary>
        /// <param name="arg"></param>
        _onAppResume: function (arg) {
            if (EventHandlerForDeviceClass.current._watcherSuspended) {
                EventHandlerForDeviceClass.current._watcherSuspended = false;

                EventHandlerForDeviceClass.current._startDeviceWatcher();
            }
        },
        /// <summary>
        /// Close the device that is opened so that all pending operations are canceled properly.
        /// </summary>
        /// <param name="deviceInformationUpdate"></param>
        _onDeviceRemoved: function (deviceInformationUpdate) {
            if ((deviceInformationUpdate.id === EventHandlerForDeviceClass.current._deviceInformation.id)
                && EventHandlerForDeviceClass.current.isDeviceConnected) {
                // The main reasons to close the device explicitly is to clean up resources, to properly handle errors,
                // and stop talking to the disconnected device.
                EventHandlerForDeviceClass.current._closeCurrentlyConnectedDevice();
            }
        },
        /// <summary>
        /// Open the device that the user wanted to open if it hasn't been opened yet and auto reconnect is enabled.
        /// </summary>
        /// <param name="deviceInformation"></param>
        _onDeviceAdded: function (deviceInformation) {
            if ((deviceInformation.id === EventHandlerForDeviceClass.current._deviceInformation.id)
                && !EventHandlerForDeviceClass.current.isDeviceConnected && EventHandlerForDeviceClass.current.isEnabledAutoReconnect) {
                EventHandlerForDeviceClass.current.openDeviceAsync(EventHandlerForDeviceClass.current._deviceInformation, EventHandlerForDeviceClass.current._deviceSelector);

                // Any app specific device intialization should be done here because we don't know the state of the device when it is re-enumerated.
            }
        },
    }, {
        _current: null,
        /// <summary>
        /// Enforces the singleton pattern so that there is only one object handling app events
        /// as it relates to the UsbDevice because this sample app only supports communicating with one device at a time. 
        ///
        /// An instance of EventHandlerForDevice is globally available because the device needs to persist across scenario pages.
        ///
        /// If there is no instance of EventHandlerForDevice created before this property is called,
        /// an EventHandlerForDevice will be created; the EventHandlerForDevice created this way
        /// is not meant for BackgroundTasks.
        /// </summary>
        current: {
            get: function () {
                if (!this._current) {
                    EventHandlerForDeviceClass.createNewEventHandlerForDevice();
                }

                return this._current;
            }
        },
        /// <summary>
        /// Creates a new instance of EventHandlerForDevice, enables auto reconnect, and uses it as the Current instance.
        /// </summary>
        createNewEventHandlerForDevice: function () {
            EventHandlerForDeviceClass._current = new EventHandlerForDeviceClass(false);
        },
        /// <summary>
        /// Creates a new instance of EventHandlerForDevice, disables auto reconnect, and uses it as the Current instance
        /// </summary>
        createNewEventHandlerForDeviceForBackgroundTasks: function () {
            EventHandlerForDeviceClass._current = new EventHandlerForDeviceClass(true);
        }
    });

    WinJS.Namespace.define(SdkSample.Constants.sampleNamespace, {
        eventHandlerForDevice: EventHandlerForDeviceClass
    });
})();