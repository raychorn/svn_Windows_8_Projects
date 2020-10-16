//// Copyright (c) Microsoft Corporation. All rights reserved

(function () {

    var LoggingScenario = WinJS.Class.define(
        function () {
            Windows.UI.WebUI.WebUIApplication.addEventListener("suspending", this._onAppSuspension);
            Windows.UI.WebUI.WebUIApplication.addEventListener("resuming", this._onAppResume);
        },
        {
            doScenarioAsync: function () {
                this._setBusyStatus(true);
                try {

                    var data = { loggingScenario: this, messageIndex: 0, startFileCount: this._logFileGeneratedCount, countDownToError: 5000 };

                    function logMessagesAndPauseAsync(data) {

                        var totalFilesCreatedSoFar = data.loggingScenario._logFileGeneratedCount - data.startFileCount;
                        if (totalFilesCreatedSoFar >= 3) {
                            // At least 3 files created, stop.
                            return new WinJS.Promise(function (completionFunction, errorFunction, progressFunction) {
                                data.loggingScenario._setBusyStatus(false);
                                completionFunction();
                            });
                        } else {

                            for (var index = 0; index < 50; index++) {

                                try {
                                    // Since the channel is added to the session at level Warning,
                                    // the following is logged because it is logged at level LoggingLevel.Critical.
                                    messageToLog =
                                        "Message=" +
                                        (++data.messageIndex) +
                                        ": Lorem ipsum dolor sit amet, consectetur adipiscing elit.In ligula nisi, vehicula nec eleifend vel, rutrum non dolor.Vestibulum ante ipsum " +
                                        "primis in faucibus orci luctus et ultrices posuere cubilia Curae; Curabitur elementum scelerisque accumsan. In hac habitasse platea dictumst.";
                                    data.loggingScenario.logMessage(messageToLog, Windows.Foundation.Diagnostics.LoggingLevel.critical);

                                    // Since the channel is added to the session at level Warning,
                                    // the following is *not* logged because it is logged at LoggingLevel.Information.
                                    messageToLog =
                                        "Message=" +
                                        (++data.messageIndex) +
                                        ": Lorem ipsum dolor sit amet, consectetur adipiscing elit.In ligula nisi, vehicula nec eleifend vel, rutrum non dolor.Vestibulum ante ipsum " +
                                        "primis in faucibus orci luctus et ultrices posuere cubilia Curae; Curabitur elementum scelerisque accumsan. In hac habitasse platea dictumst.";
                                    data.loggingScenario.logMessage(messageToLog, Windows.Foundation.Diagnostics.LoggingLevel.information);

                                    var value = 1000000; // one million, 7 digits, 4-bytes as an int, 14 bytes as a wide character string.
                                    data.loggingScenario.logMessage("Value #" + (++data.messageIndex).toString() + "  " + value, Windows.Foundation.Diagnostics.LoggingLevel.critical); // value is logged as 14 byte wide character string.
                                    data.loggingScenario.logValuePair("Value #" + (++data.messageIndex).toString(), value, Windows.Foundation.Diagnostics.LoggingLevel.critical); // value is logged as a 4-byte integer.

                                    //
                                    // Every once in a while, simulate an application error
                                    // which causes the app to save the current snapshot
                                    // of logging events in memory to a disk ETL file.
                                    //

                                    data.countDownToError--;
                                    if (data.countDownToError <= 0) {
                                        data.countDownToError = 5000;
                                        throw "Some bad app error occurred.";
                                    }
                                } catch (e) {

                                    // Log the exception string.
                                    data.loggingScenario.logMessage("Exception occurrred: " + e.toString(), Windows.Foundation.Diagnostics.LoggingLevel.error);
                                    return data.loggingScenario._saveLogInMemoryToFileAsync(data.loggingScenario._session).then(function (logFileName) {
                                        // The log containinng the error has been saved.
                                        // Update the UI of the sample.
                                        data.loggingScenario._logFileGeneratedCount++;
                                        data.loggingScenario._dispatchStatusChanged({ type: "LogFileGenerated", logFilePath: logFileName });
                                        // Continue with the scenario. 
                                        return logMessagesAndPauseAsync(data);
                                    });
                                }
                            }

                            // Pause every once in a while to simulate application 
                            // activity outside of logging.
                            return WinJS.Promise.timeout(25).then(function () {

                                return logMessagesAndPauseAsync(data);
                            });
                        }
                    }

                    // Log messages until at least 3 log files are created. 
                    return logMessagesAndPauseAsync(data);

                } catch (e) {
                    this._setBusyStatus(false);
                    throw e;
                }
            },
            toggleLoggingEnabledDisabled: function () {
                this._setBusyStatus(true);
                try {
                    var loggingScenario = this;
                    if (this._session !== null) {
                        this._session.close();
                        this._session = null;
                        Windows.Storage.ApplicationData.current.localSettings.values["LoggingEnabled"] = false;
                        loggingScenario._dispatchStatusChanged({ type: "LoggingEnabledDisabled", enabled: false });
                    } else {
                        this._startLogging();
                        Windows.Storage.ApplicationData.current.localSettings.values["LoggingEnabled"] = true;
                        loggingScenario._dispatchStatusChanged({ type: "LoggingEnabledDisabled", enabled: true });
                    }
                    loggingScenario._setBusyStatus(false);
                } catch (e) {
                    this._setBusyStatus(false);
                    throw e;
                }
            },
            logMessage: function (message, level) {

                if (this.isLoggingChannelEnabledForLevel(level)) {

                    this._channel.logMessage(message, level);

                    // Total bytes is roughly 2 bytes per wide character.
                    // This non-critical calculation is for sample UI feedback purposes.
                    this._logMessageApproximateByteCount += (message.length * 2);
                    this._incrementMessageCount();
                }
            },
            logValuePair: function (value1, value2, level) {

                if (this.isLoggingChannelEnabledForLevel(level)) {

                    this._channel.logValuePair(value1, value2, level);

                    // Total bytes is roughly 2 bytes per wide character, plus 4 bytes for the integer.
                    // This non-critical calculation is for sample UI feedback purposes.
                    this._logMessageApproximateByteCount += (value1.length * 2) + 4; 
                    this._incrementMessageCount();
                }
            },
            isLoggingChannelEnabledForLevel: function (levelToTest) {

                if (this._channel === null) {
                    // No channel, so "false," logging is not enabled for the specified level.
                    return false;
                }

                if (this._isChannelEnabled && levelToTest >= this._channelLoggingLevel) {
                    // The channel is enabled, and the caller's level is equal to or higher
                    // than the aggregate level of all the channels' listeners.
                    return true;
                }

                // No sessions consuming events from this channel, so 'false' for "not enabled."
                return false;
            },
            isLoggingEnabled: {
                get: function () {
                    return this._session !== null;
                }
            },
            _session: null,
            _channel: null,
            _isChannelEnabled: false,
            _channelLoggingLevel: Windows.Foundation.Diagnostics.LoggingLevel.verbose,
            _logFileGeneratedCount: 0,
            _logMessageCount: 0,
            getLogMessageCount: function () {
                return this._logMessageCount;
            },
            getLogMessageApproximateByteCount: function () {
                return this._logMessageApproximateByteCount;
            },
            _incrementMessageCount: function () {

                this._logMessageCount++;

                // Update any listening UI every 500 messages.
                if (this._logMessageCount % 500 === 0) {
                    this._dispatchStatusChanged({ type: "LogMessageCountUpdate" });
                }

                return this._logMessageCount;
            },
            _logMessageApproximateByteCount: 0,
            _isBusy: false,
            _setBusyStatus: function (busy) {
                if (this._isBusy === busy) {
                    return;
                }
                this._isBusy = busy;
                if (busy) {
                    this._dispatchStatusChanged({ type: "BusyStatusChanged" });
                }
            },
            getBusyStatus: function () {
                return this._isBusy;
            },            
            _onAppSuspension: function (suspendingEventArgs) {
                var loggingScenario = LoggingScenario.loggingScenarioSingleton;
                var deferral = suspendingEventArgs.suspendingOperation.getDeferral();
                loggingScenario._prepareToSuspend();
                deferral.complete();
            },
            _onAppResume: function (resmingEventArgs) {
                var loggingScenario = LoggingScenario.loggingScenarioSingleton;
                loggingScenario.resumeLoggingIfApplicable();
            },
            _startLogging: function () {

                var sessionCreated = false;
                var channelCreated = false;

                if (this._session === null)
                {
                    this._session = new Windows.Foundation.Diagnostics.LoggingSession("AppSession1");
                    sessionCreated = true;
                }

                if (this._channel === null)
                {
                    this._channel = new Windows.Foundation.Diagnostics.LoggingChannel("AppChannel1");
                    this._channel.addEventListener("loggingenabled", this._onLoggingEnabled);
                    channelCreated = true;
                }

                if (sessionCreated || channelCreated) {
                    // This sample adds the channel at level "warning" to 
                    // demonstrated how messages logged at more verbose levels
                    // are ignored by the session. 
                    this._session.addLoggingChannel(this._channel, Windows.Foundation.Diagnostics.LoggingLevel.warning);
                }
            },
            _onLoggingEnabled: function (args) {

                var loggingScenario = LoggingScenario.loggingScenarioSingleton;

                // Given the sessions consuming the channel, 
                // the channel's enablement status or level
                // has changed. Save those values.
                loggingScenario._isChannelEnabled = args.target.enabled;
                loggingScenario._channelLoggingLevel = args.target.level;
            },
            resumeLoggingIfApplicable: function () {

                // If logging is already enabled, nothing to do.
                if (this.isLoggingEnabled) {
                    return true;
                }

                var loggingEnabled;
                if (Windows.Storage.ApplicationData.current.localSettings.values.hasKey("LoggingEnabled")) {
                    loggingEnabled = Windows.Storage.ApplicationData.current.localSettings.values.hasKey("LoggingEnabled");
                } else {
                    Windows.Storage.ApplicationData.current.localSettings.values["LoggingEnabled"] = true;
                    loggingEnabled = true;
                }

                if (loggingEnabled) {
                    this._startLogging();
                }
            },
            _getLogRepositoryFolderAsync: function () {
                return Windows.Storage.ApplicationData.current.localFolder.createFolderAsync("OurAppLogFiles", Windows.Storage.CreationCollisionOption.openIfExists);
            },
            _getAllLogRepositoryFileNamesAsync: function () {
                return this._getLogRepositoryFolderAsync().then(function (logRepositoryFolder) {
                    var queryOptions = new Windows.Storage.Search.QueryOptions(Windows.Storage.Search.CommonFileQuery.orderBySearchRank, null);
                    queryOptions.applicationSearchFilter = "System.FileName:~\"OurAppLog-????.etl\"";
                    var queryResult = logRepositoryFolder.createFileQueryWithOptions(queryOptions);
                    return queryResult.getFilesAsync();
                });
            },
            _getNextLogFileNumberAsync: function () {

                // Get all files in this sample's log repository, and 
                // find the file with the highest number. Return
                // that number plus 1 as the next file number.
                return this._getAllLogRepositoryFileNamesAsync().then(function (files) {
                    var extractNumberRegExPattern = new RegExp("OurAppLog-(\\d{4})\\.etl", "i");
                    var lastNumber = 0;
                    files.forEach(function (file) {
                        var matches = extractNumberRegExPattern.exec(file.name);
                        if (matches !== null) {
                            var fileNumber = parseInt(matches[1], 10);
                            if (isNaN(fileNumber) == false && fileNumber > lastNumber) {
                                lastNumber = fileNumber;
                            }
                        }
                    });

                    // return the next available log file number.
                    return lastNumber + 1;
                });
            },
            _getNextLogFileNameAsync: function () {
                return this._getNextLogFileNumberAsync().then(function (nextLogFileNumber) {
                    return "OurAppLog-" + ("000" + nextLogFileNumber).substr(-4) + ".etl";
                });
            },
            _saveLogInMemoryToFileAsync: function (sessionToSave) {
                /// <param name="sessionToSave" type="Windows.Foundation.Diagnostics.LoggingSession">The LoggingSession.</param>
                /// <returns type="WinJS.Promise">A Promise to return the final path of the log file, or null if there was no final log file.</returns>
                
                var data = { loggingScenario: this, logRepositoryFolder: null, nextLogFileName: null };

                return this._getLogRepositoryFolderAsync().then(function (logRepositoryFolder) {
                    data.logRepositoryFolder = logRepositoryFolder;
                    return data.loggingScenario._getNextLogFileNameAsync();
                }).then(function (nextLogFileName) {
                    data.nextLogFileName = nextLogFileName;
                    return sessionToSave.saveToFileAsync(data.logRepositoryFolder, nextLogFileName);
                }).then(function (logFile) {
                    return logFile.path;
                });
            },
            _prepareToSuspend: function () {
                if (this._session !== null) {
                    Windows.Storage.ApplicationData.current.localSettings.values["LoggingEnabled"] = true;
                } else {
                    Windows.Storage.ApplicationData.current.localSettings.values["LoggingEnabled"] = false;
                }
            },
            _dispatchStatusChanged: function (statusArgs) {
                this.dispatchEvent("statusChanged", statusArgs);
            }
        },
        {
            // LoggingScenario static members:

            _loggingScenarioSingleton: null,
            loggingScenarioSingleton: {
                get: function () {
                    if (!this._loggingScenarioSingleton) {
                        this._loggingScenarioSingleton = new LoggingScenario()
                    }
                    return this._loggingScenarioSingleton;
                }
            }
        });
    
    WinJS.Class.mix(LoggingScenario, WinJS.Utilities.eventMixin);

    // Export public methods & controls
    WinJS.Namespace.define("LoggingScenario", {
        LoggingScenario: LoggingScenario
    });

})();