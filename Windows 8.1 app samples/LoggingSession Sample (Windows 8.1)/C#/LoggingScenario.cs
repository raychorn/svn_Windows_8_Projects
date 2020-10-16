//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using Windows.ApplicationModel.Activation;
using Windows.Foundation.Diagnostics;
using Windows.Storage;
using Windows.Storage.Search;

namespace LoggingSession
{
    /// <summary>
    /// LoggingScenario is a central singleton class which contains the logging-specific 
    /// sample code. 
    /// </summary>
    class LoggingScenario
    {
        #region Primary scenario code

        public bool ToggleLoggingEnabledDisabled()
        {
            IsBusy = true;
            try
            {
                bool enabled;
                Windows.Foundation.Diagnostics.LoggingSession sessionCopy = Interlocked.Exchange(ref session, null);
                if (sessionCopy != null)
                {
                    sessionCopy.Dispose();
                    ApplicationData.Current.LocalSettings.Values["LoggingEnabled"] = false;
                    enabled = false;
                }
                else
                {
                    StartLogging();
                    ApplicationData.Current.LocalSettings.Values["LoggingEnabled"] = true;
                    enabled = true;
                }

                if (StatusChanged != null)
                {
                    StatusChanged.Invoke(this, new LoggingScenarioEventArgs(enabled));
                }

                return enabled;
            }
            finally
            {
                IsBusy = false;
            }
        }

        public void LogMessage(string message, Windows.Foundation.Diagnostics.LoggingLevel level)
        {

            if (IsLoggingChannelEnabledForLevel(level))
            {
                channel.LogMessage(message, level);

                // Total bytes is roughly 2 bytes per wide character.
                // This non-critical calculation is for sample UI feedback purposes.
                LogMessageApproximateByteCount += message.Length * 2; // rough approximation for UI feedback purposes.
                LogMessageCount++;
            }
        }

        public void LogValuePair(string value1, int value2, Windows.Foundation.Diagnostics.LoggingLevel level)
        {
            if (IsLoggingChannelEnabledForLevel(level))
            {
                channel.LogValuePair(value1, value2, level);

                // Total bytes is roughly 2 bytes per wide character, plus 4 bytes for the integer.
                // This non-critical calculation is for sample UI feedback purposes.
                LogMessageApproximateByteCount += (value1.Length * 2) + 4; // rough approximation for UI feedback purposes.
                LogMessageCount++;
            }
        }

        public async Task DoScenarioAsync()
        {
            IsBusy = true;

            try
            {
                await Task.Run(async () =>
                {
                    const int NUMBER_OF_LOG_FILES_TO_GENERATE = 3;
                    int messageIndex = 0;
                    int startFileCount = LogFileGeneratedCount;

                    //
                    // Log large messages until the current log file hits the maximum size.
                    // When the current log file reaches its maximum size, LogFileGeneratedHandler 
                    // will be called.
                    //

                    while (LogFileGeneratedCount - startFileCount < NUMBER_OF_LOG_FILES_TO_GENERATE)
                    {
                        try 
                        {
                       
                            // Since the channel is added to the session at level Warning,
                            // the following is logged because it is logged at level LoggingLevel.Critical.
                            LogMessage(
                                string.Format("Message={0}: Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ligula nisi, vehicula nec eleifend vel, rutrum non dolor. Vestibulum ante ipsum " +
                                              "primis in faucibus orci luctus et ultrices posuere cubilia Curae; Curabitur elementum scelerisque accumsan. In hac habitasse platea dictumst.",
                                              ++messageIndex),
                                LoggingLevel.Critical);

                            // Since the channel is added to the session at level Warning,
                            // the following is *not* logged because it is logged at LoggingLevel.Information.
                            LogMessage(
                                string.Format("Message={0}: Lorem ipsum dolor sit amet, consectetur adipiscing elit. In ligula nisi, vehicula nec eleifend vel, rutrum non dolor. Vestibulum ante ipsum " +
                                              "primis in faucibus orci luctus et ultrices posuere cubilia Curae; Curabitur elementum scelerisque accumsan. In hac habitasse platea dictumst.",
                                              ++messageIndex),
                                LoggingLevel.Information);

                            int value = 1000000; // one million, 7 digits, 4-bytes as an int, 14 bytes as a wide character string.
                            LogMessage("Value #" + (++messageIndex).ToString() + "  " + value.ToString(), LoggingLevel.Critical); // value is logged as 14 byte wide character string.
                            LogValuePair("Value #" + (++messageIndex).ToString(), value, LoggingLevel.Critical); // value is logged as a 4-byte integer.

                            //
                            // Pause every once in a while to simulate application 
                            // activity outside of logging.
                            //

                            if (messageIndex % 50 == 0)
                            {
                                await Task.Delay(10);
                            }

                            //
                            // Every once in a while, simulate an application error
                            // which causes the app to save the current snapshot
                            // of logging events in memory to a disk ETL file.
                            //

                            if (messageIndex != 0 && messageIndex % 25000 == 0)
                            {
                                throw new AppException("Some bad app error occurred.");
                            }
                        }
                        catch (AppException e)
                        {
                            // Log the exception string.
                            LogMessage("Exception occurrred: " + e.ToString(), LoggingLevel.Error);
                            // Save the memory log buffer to file.
                            Task<string> op = SaveLogInMemoryToFileAsync(session);
                            op.Wait();
                            if (op.IsFaulted)
                            {
                                throw new Exception("After an app error occurred, there was a failure to save the log file.", op.Exception);
                            }
                            LogFileGeneratedCount++;
                            // For the sample, update the UI to show a log file has been generated.
                            if (StatusChanged != null)
                            {
                                StatusChanged.Invoke(this, new LoggingScenarioEventArgs(LoggingScenarioEventType.LogFileGenerated, op.Result));
                            }
                        }
                    }
                });
            }
            finally
            {
                IsBusy = false;
            }
        }

        public void StartLogging()
        {
            bool sessionCreated = false;
            bool channelCreated = false;

            // StartLogging called in two places: resume and when logging is 
            // enabled. It is assumed both cannot occur at the same time so no
            // interlocked or synchronization is used when checking 'session' here. 
            if (session == null)
            {
                session = new Windows.Foundation.Diagnostics.LoggingSession(DEFAULT_SESSION_NAME);
                sessionCreated = true;
            }

            if (channel == null)
            {
                channel = new Windows.Foundation.Diagnostics.LoggingChannel(DEFAULT_CHANNEL_NAME);
                channel.LoggingEnabled += OnChannelLoggingEnabled;
                channelCreated = true;
            }

            if (sessionCreated || channelCreated)
            {
                session.AddLoggingChannel(channel, LoggingLevel.Warning);
            }
        }

        private async Task<string> SaveLogInMemoryToFileAsync(Windows.Foundation.Diagnostics.LoggingSession sessionToCloseAndSave)
        {
            StorageFolder logRepositoryFolder =
                await ApplicationData.Current.LocalFolder.CreateFolderAsync(LOG_FILE_REPOSITORY_FOLDER_NAME,
                                                                            CreationCollisionOption.OpenIfExists);
            string nextLogFileNameToUse = await GetNextLogFileNameAsync();
            StorageFile finalFileBeforeSuspend = await sessionToCloseAndSave.SaveToFileAsync(logRepositoryFolder, nextLogFileNameToUse);
            if (finalFileBeforeSuspend != null)
            {
                return finalFileBeforeSuspend.Path;
            }
            else
            {
                return null;
            }
        }

        #endregion

        #region Scenario code for tracking a LoggingChannel's enablement status and related logging level.

        /// <summary>
        /// This boolean tracks whether or not there are any
        /// sessions listening to the app's channel. This is
        /// adjusted as the channel's LoggingEnabled event is 
        /// raised. Search for OnChannelLoggingEnabled for 
        /// more information.
        /// </summary>
        private bool isChannelEnabled = false;

        /// <summary>
        /// This is the current maximum level of listeners of
        /// the application's channel. It is adjusted as the 
        /// channel's LoggingEnabled event is raised. Search
        /// for OnChannelLoggingEnabled for more information.
        /// </summary>
        private Windows.Foundation.Diagnostics.LoggingLevel channelLoggingLevel = LoggingLevel.Verbose;

        void OnChannelLoggingEnabled(ILoggingChannel sender, object args)
        {
            isChannelEnabled = sender.Enabled;
            channelLoggingLevel = sender.Level;
        }

        public bool IsLoggingChannelEnabledForLevel(Windows.Foundation.Diagnostics.LoggingLevel level)
        {
            if (channel == null)
            {
                //
                // No channel, so "false" logging is not enabled for the specified level.
                //

                return false;
            }

            if (isChannelEnabled && level >= channelLoggingLevel)
            {
                //
                // The channel is enabled, and the caller's level is equal to or higher
                // than the aggregate level of all the channel's listeners.
                //

                return true;
            }

            //
            // No sessions consuming events from this channel, so 'false' for "not enabled."
            //

            return false;
        }

        #endregion

        #region Scenario code for moving generated logs to an app-defined "log repository" folder.

        /// <summary>
        /// Get all log file names in the app's log file repository.
        /// </summary>
        /// <returns>An array of log file names.</returns>
        private async Task<StorageFile[]> GetAllLogRepositoryFileNamesAsync()
        {
            StorageFolder logRepositoryFolder =
                await ApplicationData.Current.LocalFolder.CreateFolderAsync(LOG_FILE_REPOSITORY_FOLDER_NAME,
                                                                            CreationCollisionOption.OpenIfExists);
            QueryOptions queryOptions = new QueryOptions(CommonFileQuery.OrderBySearchRank, null);
            queryOptions.ApplicationSearchFilter = string.Format("System.FileName:~\"{0}-????.etl\"", LOG_FILE_BASE_FILE_NAME);
            StorageFileQueryResult queryResult = logRepositoryFolder.CreateFileQueryWithOptions(queryOptions);
            IReadOnlyList<StorageFile> files = await queryResult.GetFilesAsync();
            return files.ToArray();
        }

        /// <summary>
        /// Get the next log file number.
        /// </summary>
        /// <returns>The next log file number</returns>
        private async Task<int> GetNextLogFileNumberAsync()
        {
            // Extract each log file's number to determine the next number to use.
            StorageFile[] files = await GetAllLogRepositoryFileNamesAsync();
            Regex extractNumberRegEx = new Regex(string.Format("{0}-(\\d{{4}})\\.etl", LOG_FILE_BASE_FILE_NAME), RegexOptions.IgnoreCase);
            int lastNumber = 0;
            foreach (StorageFile file in files)
            {
                Match m = extractNumberRegEx.Match(file.Name);
                int tempNumber;
                if (m.Success && int.TryParse(m.Groups[1].Value, out tempNumber) == true && tempNumber > lastNumber)
                {
                    lastNumber = tempNumber;
                }
            }

            return lastNumber + 1;
        }

        private async Task<string> GetNextLogFileNameAsync()
        {
            int nextLogFileNumber = await GetNextLogFileNumberAsync();
            return string.Format("{0}-{1:D4}.etl", LOG_FILE_BASE_FILE_NAME, nextLogFileNumber);
        }

        #endregion

        #region Scenario code for suspend/resume.

        public bool IsSuspending { get; private set; }
        public bool IsLoggingEnabled
        {
            get { return session != null; }
        }

        public void PrepareToSuspend()
        {
            if (session != null)
            {
                IsSuspending = true;

                try
                {
                    ApplicationData.Current.LocalSettings.Values["LoggingEnabled"] = true;
                }
                finally
                {
                    IsSuspending = false;
                }
            }
            else
            {
                ApplicationData.Current.LocalSettings.Values["LoggingEnabled"] = false;
            }
        }

        public void ResumeLoggingIfApplicable()
        {
            object loggingEnabled;
            if (ApplicationData.Current.LocalSettings.Values.TryGetValue("LoggingEnabled", out loggingEnabled) == false)
            {
                ApplicationData.Current.LocalSettings.Values["LoggingEnabled"] = true;
                loggingEnabled = ApplicationData.Current.LocalSettings.Values["LoggingEnabled"];
            }

            if (loggingEnabled is bool && (bool)loggingEnabled == true)
            {
                StartLogging();
            }
        }

        #endregion

        #region Helper functions/properties/events to support sample UI feedback.

        public event EventHandler<LoggingScenarioEventArgs> StatusChanged;

        private bool isBusy;
        public bool IsBusy
        {
            get
            {
                return isBusy;
            }

            private set
            {
                isBusy = value;
                if (StatusChanged != null)
                {
                    StatusChanged.Invoke(this, new LoggingScenarioEventArgs(LoggingScenarioEventType.BusyStatusChanged));
                }
            }
        }

        private int logMessageCount = 0;
        public int LogMessageCount
        {
            get
            {
                return logMessageCount;
            }

            private set
            {
                logMessageCount = value;
                if (logMessageCount % 500 == 0 && StatusChanged != null)
                {
                    StatusChanged.Invoke(this, new LoggingScenarioEventArgs(LoggingScenarioEventType.LogMessageCountUpdate));
                }
            }
        }

        public int LogMessageApproximateByteCount { get; private set; }

        /// <summary>
        /// The number of times LogFileGeneratedHandler has been called.
        /// </summary>

        public int LogFileGeneratedCount { get; private set; }

        #endregion

        #region LoggingScenario constants and privates.

        public const string DEFAULT_SESSION_NAME = "AppSession1";
        public const string DEFAULT_CHANNEL_NAME = "AppChannel1";

        /// <summary>
        /// LoggingScenario moves generated logs files into the 
        /// this folder under the LocalState folder.
        /// </summary>
        public const string LOG_FILE_REPOSITORY_FOLDER_NAME = "OurAppLogFiles";

        /// <summary>
        /// A base name to use when moving generated log files into the app's log file folder.
        /// </summary>
        private const string LOG_FILE_BASE_FILE_NAME = "OurAppLog";

        /// <summary>
        /// <summary>
        /// The sample's one session.
        /// </summary>
        private Windows.Foundation.Diagnostics.LoggingSession session;

        /// <summary>
        /// The sample's one channel.
        /// </summary>
        private Windows.Foundation.Diagnostics.LoggingChannel channel;

        /// <summary>
        /// A semaphore to limit log file moving to one thread at a time. 
        /// </summary>
        private SemaphoreSlim moveLogSemaphore = new SemaphoreSlim(1);

        #endregion

        #region LoggingScenario constructor and singleton accessor.

        /// <summary>
        /// Disallow creation of instances beyond the one instance for the process.
        /// The one instance is accessible via GetLoggingScenarioSingleton() (see below).
        /// </summary>
        private LoggingScenario()
        {
            LogFileGeneratedCount = 0;
            LogMessageCount = 0;

            SDKTemplate.App.Current.Suspending += OnAppSuspending;
            SDKTemplate.App.Current.Resuming += OnAppResuming;

            // If the app is being launched (not resumed), the 
            // following call will activate logging if it had been
            // activated during the last suspend. 
            ResumeLoggingIfApplicable();
        }

        void OnAppSuspending(object sender, Windows.ApplicationModel.SuspendingEventArgs e)
        {
            var deferral = e.SuspendingOperation.GetDeferral();
            PrepareToSuspend();
            deferral.Complete();
        }

        void OnAppResuming(object sender, object e)
        {
            // If logging was active at the last suspend,
            // ResumeLoggingIfApplicable will re-activate 
            // logging.
            ResumeLoggingIfApplicable();
        }

        /// The app's one and only LoggingScenario instance.
        /// </summary>
        static private LoggingScenario loggingScenario;

        /// <summary>
        /// A method to allowing callers to access the app's one and only LoggingScenario instance.
        /// </summary>
        /// <returns>The logging helper.</returns>
        static public LoggingScenario GetLoggingScenarioSingleton()
        {
            if (loggingScenario == null)
            {
                loggingScenario = new LoggingScenario();
            }
            return loggingScenario;
        }

        #endregion
    }

    #region Exception to simulate an app error.

    class AppException : Exception
    {
        public AppException()
        {

        }

        public AppException(string message) : base(message)
        {

        }
    }

    #endregion 

    #region Sample UI feedback event args.

    enum LoggingScenarioEventType
    {
        BusyStatusChanged,
        LogFileGenerated,
        LoggingEnabledDisabled,
        LogMessageCountUpdate
    }

    class LoggingScenarioEventArgs : EventArgs
    {
        public LoggingScenarioEventArgs(LoggingScenarioEventType type)
        {
            Type = type;
        }

        public LoggingScenarioEventArgs(LoggingScenarioEventType type, string logFilePath)
        {
            Type = type;
            LogFilePath = logFilePath;
        }

        public LoggingScenarioEventArgs(bool enabled)
        {
            Type = LoggingScenarioEventType.LoggingEnabledDisabled;
            Enabled = enabled;
        }

        public LoggingScenarioEventType Type { get; private set; }
        public string LogFilePath { get; private set; }
        public bool Enabled { get; private set; }
    }

    #endregion
}
