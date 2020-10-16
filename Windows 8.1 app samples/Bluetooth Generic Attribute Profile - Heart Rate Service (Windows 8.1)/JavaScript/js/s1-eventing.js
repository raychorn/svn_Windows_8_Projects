//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";

    var page = WinJS.UI.Pages.define("/html/s1-eventing.html", {
        ready: function (element, options) {
            refreshButton.addEventListener("click", onRefreshButtonClick, false);
            document.addEventListener("onValueChanged", onValueChanged, false);
            HeartRateService.initializeHeartRateService();
        },
        unload: function (element, options) {
            document.removeEventListener("onValueChanged", onValueChanged, false);
        }
    });

    var dataChart = new Chart.renderer();

    function onRefreshButtonClick() {
        HeartRateService.initializeHeartRateService();
    }

    function onValueChanged(args) {
        var heartRateMeasurement = args.detail.value.heartRateValue;

        var data = HeartRateService.getData();

        dataChart.plot("outputDataChart", data);

        var measurementElement = document.createElement("option");
        measurementElement.innerText =  data[data.length - 1].toString();
        outputSelect.appendChild(measurementElement);

        document.getElementById("latestHeartRateMeasurementValue").innerText = 
			"Latest received heart rate measurement: " + heartRateMeasurement.toString();
    }
})();
