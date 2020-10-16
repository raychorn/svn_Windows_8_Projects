//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/scenario1.html", {
        ready: function (element, options) {
            initialize();
            window.addEventListener("resize", initialize, false);
            Windows.Graphics.Display.DisplayInformation.getForCurrentView().addEventListener("dpichanged", initialize);
        },
        unload: function () {
            window.removeEventListener("resize", initialize, false);
            Windows.Graphics.Display.DisplayInformation.getForCurrentView().removeEventListener("dpichanged", initialize);
        }
    });

    function initialize() {
        var displayInformation = Windows.Graphics.Display.DisplayInformation.getForCurrentView();
        var scale = displayInformation.resolutionScale;
        document.getElementById("scalePercentValue").textContent = scale + "%";
        // Get the logical DPI and round to the nearest tenth
        var logicalDPI = Math.round(displayInformation.logicalDpi * 10) / 10;
        document.getElementById("logicalDPIValue").textContent = logicalDPI + " DPI";
        document.getElementById("minPhysicalDPIValue").textContent = getMinDPIForScale(scale);
        document.getElementById("minPhysicalResolutionValue").textContent = getMinResolutionForScale(scale);
    }

    // Return the minimum native DPI needed for a device to be in the current scale
    function getMinDPIForScale(resolutionScale) {
        var dpiRange = "";

        switch (resolutionScale) {
            case 100:
                // Scale 100%
                dpiRange = "No minimum DPI for this scale";
                break;
            case 140:
                // Scale 140%
                dpiRange = "174 DPI";
                break;
            case 180:
                // Scale 180%
                dpiRange = "240 DPI";
                break;
            default:
                dpiRange = "Unknown";
                break;
        }
        return dpiRange;
    }

    // Return the minimum native resolution needed for a device to be in the current scale
    function getMinResolutionForScale(resolutionScale) {
        var resolutionRange = "";

        switch (resolutionScale) {
            case 100:
                // Scale 100%
                resolutionRange = "1024x768 (min resolution needed to run apps)";
                break;
            case 140:
                // Scale 140%
                resolutionRange = "1440x1080";
                break;
            case 180:
                // Scale 180%
                resolutionRange = "1920x1440";
                break;
            default:
                resolutionRange = "Unknown";
                break;
        }
        return resolutionRange;
    }
})();