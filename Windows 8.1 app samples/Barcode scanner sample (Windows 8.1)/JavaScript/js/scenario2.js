//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {
    "use strict";
    var page = WinJS.UI.Pages.define("/html/scenario2.html", {
        ready: function (element, options) {
            document.getElementById("ScenarioStartScanningInstance1").addEventListener("click", buttonStartScanningInstance1, false);
            document.getElementById("ScenarioEndScanningInstance1").addEventListener("click", buttonEndScanningInstance1, false);
            document.getElementById("ScenarioStartScanningInstance1").disabled = false;
            document.getElementById("ScenarioEndScanningInstance1").disabled = true;

            document.getElementById("ScenarioStartScanningInstance2").addEventListener("click", buttonStartScanningInstance2, false);
            document.getElementById("ScenarioEndScanningInstance2").addEventListener("click", buttonEndScanningInstance2, false);
            document.getElementById("ScenarioStartScanningInstance2").disabled = false;
            document.getElementById("ScenarioEndScanningInstance2").disabled = true;
        },

        unload: function () {
            if (_claimedScanner1 !== null) {
                _claimedScanner1.removeEventListener("datareceived", onDataReceived);
                _claimedScanner1.removeEventListener("releasedevicerequested", onReleasedeviceRequested1);
                _claimedScanner1.close();
                _claimedScanner1 = null;
            }

            if (_claimedScanner2 !== null) {
                _claimedScanner2.removeEventListener("datareceived", onDataReceived);
                _claimedScanner2.removeEventListener("releasedevicerequested", onReleasedeviceRequested2);
                _claimedScanner2.close();
                _claimedScanner2 = null;
            }
            _scanner1 = null;
            _scanner2 = null;
        }
    });

    var _activeBarcodeScannerInstance = { instance1: 0, instance2: 1 };
    var _scanner1 = null;
    var _claimedScanner1 = null;
    var _scanner2 = null;
    var _claimedScanner2 = null;

    var _currentInstance;
    var _retain1 = true;
    var _retain2 = true;

    function buttonStartScanningInstance1() {
        createScanner(_activeBarcodeScannerInstance.instance1).then(function (success) {
            if (success === true) {
                claimAndEnableScanner(_activeBarcodeScannerInstance.instance1).then(function (bAsyncCallStatus) {
                    if (bAsyncCallStatus === true) {
                        resetUI();
                        document.getElementById("ScenarioStartScanningInstance1").disabled = true;
                        document.getElementById("ScenarioStartScanningInstance2").disabled = false;
                        document.getElementById("ScenarioEndScanningInstance1").disabled = false;
                        document.getElementById("ScenarioEndScanningInstance2").disabled = true;
                        _currentInstance = _activeBarcodeScannerInstance.instance1;
                    }
                });
            }
        });
    }



    function buttonStartScanningInstance2() {
        createScanner(_activeBarcodeScannerInstance.instance2).then(function (success) {
            if (success === true) {
                claimAndEnableScanner(_activeBarcodeScannerInstance.instance2).then(function (bAsyncCallStatus) {
                    if (bAsyncCallStatus === true) {
                        resetUI();
                        document.getElementById("ScenarioStartScanningInstance1").disabled = false;
                        document.getElementById("ScenarioStartScanningInstance2").disabled = true;
                        document.getElementById("ScenarioEndScanningInstance1").disabled = true;
                        document.getElementById("ScenarioEndScanningInstance2").disabled = false;
                        _currentInstance = _activeBarcodeScannerInstance.instance2;
                    }
                });
            }
        });
    }

    function buttonEndScanningInstance1() {
        disableScannerAsync1().done(function (success) {

            if (success === true) {
                resetUI();
                document.getElementById("ScenarioStartScanningInstance2").disabled = false;
                document.getElementById("ScenarioStartScanningInstance1").disabled = false;
                document.getElementById("ScenarioEndScanningInstance1").disabled = true;
                document.getElementById("ScenarioEndScanningInstance2").disabled = true;
            }
        });
    }
    function buttonEndScanningInstance2() {

        disableScannerAsync2().done(function (success) {

            if (success === true) {
                resetUI();
                document.getElementById("ScenarioStartScanningInstance2").disabled = false;
                document.getElementById("ScenarioStartScanningInstance1").disabled = false;
                document.getElementById("ScenarioEndScanningInstance1").disabled = true;
                document.getElementById("ScenarioEndScanningInstance2").disabled = true;
            }
        });
    }
    function resetUI() {

        switch (_currentInstance) {

            case _activeBarcodeScannerInstance.instance1:

                document.getElementById("ScanDataType1").textContent = "No Data";
                document.getElementById("ScanData1").textContent = "No Data";
                document.getElementById("DataLabel1").textContent = "No Data";
                break;

            case _activeBarcodeScannerInstance.instance2:

                document.getElementById("ScanDataType2").textContent = "No Data";
                document.getElementById("ScanData2").textContent = "No Data";
                document.getElementById("DataLabel2").textContent = "No Data";
                break;
        }
    }

    function onReleasedeviceRequested1(args) {
        WinJS.logAppend("ReleaseDeviceRequested ("+ _claimedScanner1.deviceId +")", "status");
        var retain = document.getElementById("Retain1");        
        if (retain.checked === true) {
            try {
                _claimedScanner1.retainDevice();
                WinJS.logAppend("(Scanner Retained)", "status");
            }
            catch (error) {
                WinJS.logAppend("(retain failed) (" + error.message +")", "error");
            }
        }      
    }

    function onReleasedeviceRequested2(args) {
        WinJS.logAppend("ReleaseDeviceRequested (" + _claimedScanner2.deviceId + ")", "status");        
        var retain = document.getElementById("Retain2");
        if (retain.checked === true) {
            try {
                _claimedScanner2.retainDevice();
                WinJS.logAppend("(Scanner Retained)", "status");
            }
            catch (error) {
                WinJS.logAppend("(retain failed) (" + error.message + ")", "error");
            }
        }
    }

    function claimAndEnableScanner(instance) {

        return new WinJS.Promise(function (complete) {
            switch (instance) {
                case _activeBarcodeScannerInstance.instance1:

                    if (_scanner1 !== null) {
                        _scanner1.claimScannerAsync().done(function (claimedScanner) {
                            if (claimedScanner !== null) {
                                _claimedScanner1 = claimedScanner;
                                claimedScanner.isDecodeDataEnabled = true;
                                WinJS.logAppend("Instance1 Claim Barcode Scanner succeeded.", "status");
                                claimedScanner.addEventListener("datareceived", onDataReceived);
                                claimedScanner.addEventListener("releasedevicerequested", onReleasedeviceRequested1);
                                claimedScanner.enableAsync().done(function () {
                                    WinJS.logAppend("Instance1 Enable Barcode Scanner succeeded.", "status");
                                    WinJS.logAppend("Ready to Scan.", "status");
                                    complete(true);
                                }, function error(e) {
                                    WinJS.logAppend("Instance1 Claim Barcode Scanner failed." + e.message, "error");
                                    complete(false);
                                });
                            } else {
                                WinJS.logAppend("Instance1 Claim Barcode Scanner failed.", "error");
                                complete(false);
                            }
                        }, function error(e) {
                            WinJS.logAppend("Instance1 Claim Barcode Scanner failed." + e.message, "error");
                            complete(false);
                        });
                    }
                    break;
                case _activeBarcodeScannerInstance.instance2:
                    if (_scanner2 !== null) {
                        _scanner2.claimScannerAsync().done(function (claimedScanner) {
                            if (claimedScanner !== null) {
                                _claimedScanner2 = claimedScanner;
                                claimedScanner.isDecodeDataEnabled = true;
                                WinJS.logAppend("Instance2 Claim Barcode Scanner succeeded.", "status");
                                claimedScanner.addEventListener("datareceived", onDataReceived);
                                claimedScanner.addEventListener("releasedevicerequested", onReleasedeviceRequested2);
                                claimedScanner.enableAsync().done(function () {
                                    WinJS.logAppend("Instance2 Enable Barcode Scanner succeeded.", "status");
                                    WinJS.logAppend("Ready to Scan.", "status");
                                    complete(true);
                                }, function error(e) {
                                    WinJS.logAppend("Instance2 Claim Barcode Scanner failed." + e.message, "error");
                                    complete(false);
                                });
                            } else {
                                WinJS.logAppend("Instance2 Claim Barcode Scanner failed.", "error");
                                complete(false);
                            }
                        }, function error(e) {
                            WinJS.logAppend("Instance2 Claim Barcode Scanner failed." + e.message, "error");
                            complete(false);
                        });
                    }
                    break;
            }
        });
    }

    function createScanner(instance) {

        return new WinJS.Promise(function (complete) {
            Windows.Devices.PointOfService.BarcodeScanner.getDefaultAsync().done(function (scanner) {
                if (scanner !== null) {

                    switch (instance) {
                        case _activeBarcodeScannerInstance.instance1:
                            _scanner1 = scanner;
                            WinJS.logAppend("Instance1 Default Barcode Scanner created..", "status");
                            break;
                        case _activeBarcodeScannerInstance.instance2:
                            _scanner2 = scanner;
                            WinJS.logAppend("Instance2 Default Barcode Scanner created..", "status");
                            break;
                    }
                    complete(true);

                } else {
                    WinJS.logAppend("Scanner not found. Please connect a Barcode Scanner.", "error");
                    complete(false);
                }

            }, function error(e) {
                WinJS.logAppend("Scanner GetDefault Async Unsuccessful" + e.message, "error");
                complete(false);
            });

        });
    }

  

    /// <summary>
    /// Disables the scanner Instance1.
    /// </summary>      
    /// <returns></returns>
    function disableScannerAsync1() {
        return new WinJS.Promise(function (complete) {
            if (_claimedScanner1 !== null) {
                _claimedScanner1.disableAsync().then(function (success) {
                    _claimedScanner1.removeEventListener("datareceived", onDataReceived);
                    _claimedScanner1.removeEventListener("releasedevicerequested", onReleasedeviceRequested1);
                    _claimedScanner1.close();
                    _claimedScanner1 = null;
                    _scanner1 = null;
                    resetUI();
                    WinJS.logAppend("Scanner Instance 1 Destroyed", "status");
                    complete(true);
                });
            }
        });
    }

    /// <summary>
    /// Disables the scanner Instance2.
    /// </summary>      
    /// <returns></returns>
    function disableScannerAsync2() {
        return new WinJS.Promise(function (complete) {
            if (_claimedScanner2 !== null) {
                _claimedScanner2.disableAsync().then(function (success) {
                    _claimedScanner2.removeEventListener("datareceived", onDataReceived);
                    _claimedScanner2.removeEventListener("releasedevicerequested", onReleasedeviceRequested2);
                    _claimedScanner2.close();
                    _claimedScanner2 = null;
                    _scanner2 = null;
                    resetUI();
                    WinJS.logAppend("Scanner Instance 2 Destroyed", "status");
                    complete(true);
                });
            }
        });
    }
    function onDataReceived(args) {
        var tempScanLabel = "";
        var tempScanData = "";

        if (args.report.scanData !== null) {
            tempScanData = Windows.Storage.Streams.DataReader.fromBuffer(args.report.scanData).readString(args.report.scanData.length);
        }
        if (args.report.scanDataLabel !== null) {
            tempScanLabel = Windows.Storage.Streams.DataReader.fromBuffer(args.report.scanDataLabel).readString(args.report.scanDataLabel.length);
        }   

        // Now populate the UI
        switch (_currentInstance) {
            case _activeBarcodeScannerInstance.instance1:
                if (args.report.scanDataType === 501) {
                    document.getElementById("ScanDataType1").textContent = "OEM - " + args.report.scanDataType.toString();
                } else {
                    document.getElementById("ScanDataType1").textContent = args.report.scanDataType.toString();
                }
                // DataLabel
                document.getElementById("DataLabel1").textContent = tempScanLabel;
                // Data
                document.getElementById("ScanData1").textContent = tempScanData;

                break;

            case _activeBarcodeScannerInstance.instance2:
                // BarcodeSymbologies.ExtendedBase
                if (args.report.scanDataType === 501) {
                    document.getElementById("ScanDataType2").textContent = "OEM - " + args.report.scanDataType.toString();
                } else {
                    document.getElementById("ScanDataType2").textContent = args.report.scanDataType.toString();
                }
                // DataLabel
                document.getElementById("DataLabel2").textContent = tempScanLabel;
                // Data
                document.getElementById("ScanData2").textContent = tempScanData;
                break;
        }
    }

 
})();
