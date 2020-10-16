//// Copyright (c) Microsoft Corporation. All rights reserved

(

function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/scenario1.html",

        {
            ready: function (element, options) {
                document.getElementById("ScenarioStartScanButton").addEventListener("click", startReceivingData, false);
                document.getElementById("ScenarioEndScanButton").addEventListener("click", endReceivingData, false);
                document.getElementById("ScenarioStartScanButton").disabled = false;
                document.getElementById("ScenarioEndScanButton").disabled = true;
            },

            unload: function () {
                if (document.getElementById("ScenarioStartScanButton").disabled) {
                    if (_claimedScanner !== null) {
                        _claimedScanner.removeEventListener("datareceived", onDataReceived);
                        _claimedScanner.removeEventListener("releasedevicerequested", onReleasedeviceRequested);
                        _claimedScanner.close();
                        _claimedScanner = null;
                    }
                    _scanner = null;
                }
            }
        });
    var _scanner = null;
    var _claimedScanner = null;

    function startReceivingData() {
        Windows.Devices.PointOfService.BarcodeScanner.getDefaultAsync().then(function (scanner) {
            if (scanner !== null) {
                _scanner = scanner;
                WinJS.logAppend("Default Barcode Scanner created..",  "status");
                WinJS.logAppend("Device Id is:" + scanner.deviceId,  "status");
                scanner.claimScannerAsync().done(function (claimedScanner) {
                    if (claimedScanner !== null) {
                        _claimedScanner = claimedScanner;
                        claimedScanner.isDecodeDataEnabled = true;
                        WinJS.logAppend("Claim Barcode Scanner succeeded..",  "status");
                        claimedScanner.addEventListener("datareceived", onDataReceived);
                        claimedScanner.addEventListener("releasedevicerequested", onReleasedeviceRequested);
                        claimedScanner.enableAsync().done(function () {
                            WinJS.logAppend("Enable Barcode Scanner succeeded..",  "status");
                            WinJS.logAppend("Ready to Scan...",  "status");
                            document.getElementById("ScenarioStartScanButton").disabled = true;
                            document.getElementById("ScenarioEndScanButton").disabled = false;
                        }, function error(e) {
                            WinJS.logAppend("Error enabling scanner..." + e.message,  "error");
                        });

                    }else {
                        WinJS.logAppend("Could not claim the scanner.",  "error");
                    }
                }, function error(e) {
                    WinJS.logAppend("Could not claim the scanner." + e.message,  "error");
                });

            }else {
                WinJS.logAppend("Barcode Scanner not found. Please connect a Barcode Scanner..",  "error");
            }

        }, function error(e) {
            WinJS.logAppend("Scanner GetDefault Async Unsuccessful" + e.message,  "error");
        });
    }

    function onReleasedeviceRequested(args) {
        _claimedScanner.retainDevice();
    }

    function onDataReceived(args) {
        var tempScanLabel = Windows.Storage.Streams.DataReader.fromBuffer(args.report.scanDataLabel).readString(args.report.scanDataLabel.length);
        var tempScanData = Windows.Storage.Streams.DataReader.fromBuffer(args.report.scanData).readString(args.report.scanData.length);
        var tempScanType = args.report.scanDataType;
       
        document.getElementById("ScenarioOutputScanDataType").textContent = tempScanType;
        document.getElementById("ScenarioOutputScanData").textContent = tempScanData;
        document.getElementById("ScenarioOutputScanDataLabel").textContent = tempScanLabel;
    }
    function endReceivingData() {
        if (_claimedScanner !== null) {
            _claimedScanner.removeEventListener("datareceived", onDataReceived);
            _claimedScanner.removeEventListener("releasedevicerequested", onReleasedeviceRequested);            
            _claimedScanner.close();
            _claimedScanner = null;
        }
        _scanner = null;
        WinJS.logAppend("Click the Start Receiving Data Button..", "status");
        document.getElementById("ScenarioStartScanButton").disabled = false;
        document.getElementById("ScenarioEndScanButton").disabled = true;
        document.getElementById("ScenarioOutputScanDataType").textContent = "No Data";
        document.getElementById("ScenarioOutputScanData").textContent = "No Data";
        document.getElementById("ScenarioOutputScanDataLabel").textContent = "No Data";
       
    }


})();
