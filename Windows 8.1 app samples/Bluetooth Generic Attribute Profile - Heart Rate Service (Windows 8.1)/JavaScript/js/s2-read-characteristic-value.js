//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    
    var page = WinJS.UI.Pages.define("/html/s2-read-characteristic-value.html", {
        ready: function (element, options) {
            document.getElementById("readValueButton").addEventListener("click", onReadValueButtonClick, false);
        }
    });

    function onReadValueButtonClick() {
        var gatt = Windows.Devices.Bluetooth.GenericAttributeProfile;
        var bodySensorLocationCharacteristics = HeartRateService.getHeartRateService().getCharacteristics(
            gatt.GattCharacteristicUuids.bodySensorLocation);

        if (bodySensorLocationCharacteristics.length > 0) {
            bodySensorLocationCharacteristics[0].readValueAsync().done(function (readResult) {
                if (readResult.status === gatt.GattCommunicationStatus.success) {
                    var bodySensorLocationData = new Uint8Array(readResult.value.length);
                    Windows.Storage.Streams.DataReader.fromBuffer(readResult.value).readBytes(bodySensorLocationData);
                    var bodySensorLocation = HeartRateService.processBodySensorLocation(bodySensorLocationData);
                    if (bodySensorLocation !== "") {
                        document.getElementById("output").innerText = "The Body Sensor Location of your device is : " +
                            bodySensorLocation;
                    } else {
                        document.getElementById("output").innerText = "The Body Sensor Location cannot be interpreted";
                    }
                }
            }, function (error) {
                WinJS.log && WinJS.log("Reading the Body Sensor Location characteristic value failed with error: " +
                    error.toString(), "sample", "error");
            });
        }

    }
})();
