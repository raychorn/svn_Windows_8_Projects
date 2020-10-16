//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    
    var page = WinJS.UI.Pages.define("/html/s3-write-characteristic-value.html", {
        ready: function (element, options) {
            writeCharacteristicValueButton.addEventListener("click", onWriteCharacteristicValueButtonClick, false);
            document.addEventListener("onValueChanged", onValueChanged, false);
            HeartRateService.initializeHeartRateService();
        },
        unload: function (element, options) {
            document.removeEventListener("onValueChanged", onValueChanged, false);
        }
    });

    function onValueChanged(args) {
        var expendedEnergy = args.detail.value.energyExpended;
        var latestExpendedEnergyValue = document.getElementById("expendedEnergyValue");

        if (expendedEnergy) {
            latestExpendedEnergyValue.innerText = "Expended Energy: " + expendedEnergy;
        } else {
            latestExpendedEnergyValue.innerText =
                "The Expended Energy value was not received when reading data from your device";
            document.removeEventListener("onValueChanged", onValueChanged, false);
        }
    }

    function onWriteCharacteristicValueButtonClick() {
        var gatt = Windows.Devices.Bluetooth.GenericAttributeProfile;

        var characteristics = HeartRateService.getHeartRateService().getCharacteristics(
            gatt.GattCharacteristicUuids.heartRateControlPoint);

        if (characteristics.length > 0) {
            var writer = new Windows.Storage.Streams.DataWriter();
            writer.writeByte(1);

            characteristics[0].writeValueAsync(writer.detachBuffer()).done(
            function (communicationStatus) {
                if (communicationStatus === gatt.GattCommunicationStatus.success) {
                    WinJS.log && WinJS.log("Expended Energy successfully reset.", "sample", "status");
                } else {
                    WinJS.log && WinJS.log("Your device is unreachable, most likely the device is out of range, " +
                        "or is running low on battery, please make sure your device is working and try again.",
                        "sample", "status");
                }
            }, function (error) {
                WinJS.log && WinJS.log("Writing the Heart Rate Control Point characteristic value failed with error :" +
                    error.toString(), "sample", "error");
            });
        }
    }
})();
