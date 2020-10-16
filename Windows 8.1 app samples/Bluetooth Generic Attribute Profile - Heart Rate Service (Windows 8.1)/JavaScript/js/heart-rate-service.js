//// Copyright (c) Microsoft Corporation. All rights reserved


(function () {
    'use strict';

    // The format for the objects in the data array will be : 
	// {timestamp: , value: , expendedEnergy: , toString: }
    var data = [];

    var heartRateService;
    var heartRateServiceInitialized = false;
    var app = WinJS.Application;
	var gatt = Windows.Devices.Bluetooth.GenericAttributeProfile;

	app.addEventListener("oncheckpoint", function (args) {
        // This application is about to be suspended, 
        // the application should release all resources at this point
        heartRateService.close();
        heartRateService = null;
    });

    function processBodySensorLocationData(bodySensorLocationData) {
        var bodySensorLocationValue = bodySensorLocationData[0];

        var retval;

        switch (bodySensorLocationValue) {
            case 0:
                retval = "Other";
                break;
            case 1:
                retval = "Chest";
                break;
            case 2:
                retval = "Wrist";
                break;
            case 3:
                retval = "Finger";
                break;
            case 4:
                retval = "Hand";
                break;
            case 5:
                retval = "Ear Lobe";
                break;
            case 6:
                retval = "Foot";
                break;
            default:
                retval = "";
                break;
        }
        return retval;
    }

    function processHeartRateMeasurementData(heartRateData) {
        var HEART_RATE_VALUE_FORMAT = 0x01;
        var ENERGY_EXPENDED_STATUS = 0x08;

        var currentOffset = 0;
        var flags = heartRateData[currentOffset];
        var isHeartRateValueSizeLong = ((flags & HEART_RATE_VALUE_FORMAT) !== 0);
        var hasEnergyExpended = ((flags & ENERGY_EXPENDED_STATUS) !== 0);

        currentOffset++;

        var heartRateMeasurementValue;

        if (isHeartRateValueSizeLong) {
            heartRateMeasurementValue = (heartRateData[currentOffset + 1] << 8) + heartRateData[currentOffset];
            currentOffset += 2;
        } else {
            heartRateMeasurementValue = heartRateData[currentOffset];
            currentOffset++;
        }

        var expendedEnergyValue;

        if (hasEnergyExpended) {
            expendedEnergyValue = (heartRateData[currentOffset + 1] << 8) + heartRateData[currentOffset];
            currentOffset += 2;
        }

        return {
            heartRateValue: heartRateMeasurementValue,
            energyExpended: expendedEnergyValue
        };
    }

    function onHeartRateMeasurementValueChanged(args) {
        var heartRateData = new Uint8Array(args.characteristicValue.length);

        Windows.Storage.Streams.DataReader.fromBuffer(args.characteristicValue).readBytes(heartRateData);

        // Interpret the Heart Rate measurement value according to the Heart Rate Bluetooth Profile
        var heartRateMeasurement = processHeartRateMeasurementData(heartRateData);

        data[data.length] = {
            timestamp: args.timestamp,
            value: heartRateMeasurement.heartRateValue,
            expendedEnergy: heartRateMeasurement.energyExpended,
            toString: function () {
                return this.value + ' bpm @ ' + this.timestamp;
            }
        };

        var evt = document.createEvent("CustomEvent");
        evt.initCustomEvent("onValueChanged", true, true, {
            value: heartRateMeasurement
        });
        document.dispatchEvent(evt);
    }

    function initializeHeartRateService() {
        if (heartRateServiceInitialized) {
            return;
        }

        Windows.Devices.Enumeration.DeviceInformation.findAllAsync(
            gatt.GattDeviceService.getDeviceSelectorFromUuid(gatt.GattServiceUuids.heartRate), null)
            .done(function (services) {
                if (services.length > 0) {
                    gatt.GattDeviceService.fromIdAsync(services[0].id).done(function (firstHeartRateService) {
                        heartRateService = firstHeartRateService;
                        var heartRateMeasurementCharacteristic = firstHeartRateService.getCharacteristics(
                            gatt.GattCharacteristicUuids.heartRateMeasurement)[0];
                        heartRateMeasurementCharacteristic.addEventListener("valuechanged",
                            onHeartRateMeasurementValueChanged, false);

                        // Writing the Client Characteristic Configuration Descriptor to the device enables the 
                        // device to send data to the client
                        heartRateMeasurementCharacteristic.writeClientCharacteristicConfigurationDescriptorAsync(
                            gatt.GattClientCharacteristicConfigurationDescriptorValue.notify).done(
                            function (communicationStatus) {
                                if (communicationStatus === gatt.GattCommunicationStatus.success) {
                                    heartRateServiceInitialized = true;
                                }
                            });
                    }, function (error) {
                        WinJS.log && WinJS.log(error, "sample", "status");
                    });
                } else {
                    WinJS.log && WinJS.log("No Bluetooth Smart Heart Rate devices found. Please pair your device" +
                        " first.", "sample", "status");
                }
            }, function (error) {
                WinJS.log && WinJS.log("Finding Heart Rate devices failed with error :" + error, "sample", "error");
            });
    }

    function getHeartRateService() {
        return heartRateService;
    }

    function getData() {
        return data;
    }

    WinJS.Namespace.define('HeartRateService', {
        initializeHeartRateService: initializeHeartRateService,
        processBodySensorLocation: processBodySensorLocationData,
        getHeartRateService: getHeartRateService,
        getData: getData
    });
})();
