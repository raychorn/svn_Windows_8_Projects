//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using SDKTemplate;
using System;
using System.Threading.Tasks;

namespace FileLoggingSession
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class S1_FileLoggingSession
    {
        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;

        internal LoggingScenario LoggingScenario { get { return LoggingScenario.GetLoggingScenarioSingleton(); } } 


        public S1_FileLoggingSession()
        {
            LoggingScenario.StatusChanged += LoggingScenario_StatusChanged; 
            this.InitializeComponent();
        }

        async void LoggingScenario_StatusChanged(object sender, LoggingScenarioEventArgs e)
        {
            if (e.Type == LoggingScenarioEventType.BusyStatusChanged)
            {
                UpdateControls();
            }
            else if (e.Type == LoggingScenarioEventType.LogFileGenerated)
            {
                await AddLogFileMessageDispatch("LogFileGenerated", e.LogFilePath);
            }
            else if (e.Type == LoggingScenarioEventType.LogFileGeneratedAtSuspend)
            {
                await AddLogFileMessageDispatch("Log file at suspend", e.LogFilePath);
            }
            else if (e.Type == LoggingScenarioEventType.LogFileGeneratedAtDisable)
            {
                await AddLogFileMessageDispatch("Log file at disable",  e.LogFilePath);
            }
            else if (e.Type == LoggingScenarioEventType.LoggingEnabledDisabled)
            {
                await AddMessageDispatch(string.Format("Logging has been {0}.", e.Enabled ? "enabled" : "disabled"));
            }
            else if (e.Type == LoggingScenarioEventType.LogMessageCountUpdate)
            {
                await UpdateLogMessageCountDispatchAsync();
            }
        }

        ScrollViewer FindScrollViewer(DependencyObject depObject)
        {
            if (depObject == null)
            {
                return null;
            }

            int countThisLevel = Windows.UI.Xaml.Media.VisualTreeHelper.GetChildrenCount(depObject);
            if (countThisLevel <= 0)
            {
                return null;
            }

            for (int childIndex = 0; childIndex < countThisLevel; childIndex++)
            {
                DependencyObject childDepObject = Windows.UI.Xaml.Media.VisualTreeHelper.GetChild(depObject, childIndex);
                if (childDepObject is ScrollViewer)
                {
                    return (ScrollViewer)childDepObject;
                }

                ScrollViewer svFromChild = FindScrollViewer(childDepObject);
                if (svFromChild != null)
                {
                    return svFromChild;
                }
            }

            return null;
        }

        public void AddMessage(string message)
        {
            string messageToAdd = message + "\r\n";
            StatusMessageList.Text += messageToAdd;
            StatusMessageList.Select(StatusMessageList.Text.Length, 0);

            ScrollViewer svFind = FindScrollViewer(StatusMessageList);
            if (svFind != null)
            {
                svFind.ChangeView(null, StatusMessageList.ActualHeight, null);
            }
        }

        public async Task AddLogFileMessageDispatch(string message, string path)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.High, () =>
            {
                string finalMessage;
                if (path != null && path.Length > 0)
                {
                    finalMessage = string.Format("{0}: {1}", message, System.IO.Path.GetFileName(path));
                    AppLogFolderLabel.Visibility = Windows.UI.Xaml.Visibility.Visible;
                    AppLogFolder.Visibility = Windows.UI.Xaml.Visibility.Visible;
                    AppLogFolder.Text = System.IO.Path.GetDirectoryName(path);

                    ViewLogInfo.Visibility = Windows.UI.Xaml.Visibility.Visible;
                    ViewLogInfo.Text = string.Format("To view the contents of the ETL files:\r\n" +
                                                     "Using tracerpt to create an XML file: tracerpt.exe \"{0}\" -of XML -o LogFile.xml\r\n" +
                                                     "Using the Windows Performance Toolkit (WPT): wpa.exe \"{0}\"",
                                                     path);
                }
                else
                {
                    finalMessage = string.Format("{0}: none, nothing logged since saving the last file.", message);
                }
                AddMessage(finalMessage);
            }).AsTask();
        }

        public async Task UpdateLogMessageCountDispatchAsync()
        {
            int messageCount = LoggingScenario.GetLoggingScenarioSingleton().LogMessageCount;
            long approximateByteCount = LoggingScenario.GetLoggingScenarioSingleton().LogMessageApproximateByteCount;
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.High, () =>
            {
                LoggingStatisticsTextBlock.Text = 
                    string.Format("Total messages logged: {0:n0}   Approximate bytes logged: {1:n0}",
                                  messageCount,
                                  approximateByteCount);
            }).AsTask();

        }

        public async Task AddMessageDispatch(string message)
        {
            await Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.High, () =>
            {
                AddMessage(message);
            }).AsTask();
        }

        private void UpdateControls()
        {
            if (LoggingScenario.GetLoggingScenarioSingleton().IsLoggingEnabled)
            {
                InputTextBlock1.Text = "Logging is enabled. Click 'Disable Logging' to disable logging. With logging enabled, you can click 'Log Messages' to use the logging API to generate log files.";
                EnableDisableLoggingButton.Content = "Disable Logging";           
                if (LoggingScenario.GetLoggingScenarioSingleton().IsBusy)
                {
                    DoScenarioButton.IsEnabled = false;
                    EnableDisableLoggingButton.IsEnabled = false;
                }
                else
                {
                    DoScenarioButton.IsEnabled = true;
                    EnableDisableLoggingButton.IsEnabled = true;
                }
            }
            else
            {
                InputTextBlock1.Text = "Logging is disabled. Click 'Enable Logging' to enable logging. After you enable logging you can click 'Log Messages' to use the logging API to generate log files.";
                EnableDisableLoggingButton.Content = "Enable Logging";
                DoScenarioButton.IsEnabled = false;
                if (LoggingScenario.GetLoggingScenarioSingleton().IsBusy)
                {
                    EnableDisableLoggingButton.IsEnabled = false;
                }
                else
                {
                    EnableDisableLoggingButton.IsEnabled = true;
                }
            }
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.  The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            UpdateControls();
        }
       
        private async void EnableDisableLogging(object sender, RoutedEventArgs e)
        {
            await LoggingScenario.GetLoggingScenarioSingleton().ToggleLoggingEnabledDisabledAsync();
            UpdateControls();
        }

        private async void DoScenario(object sender, RoutedEventArgs e)
        {            
            DoScenarioButton.IsEnabled = false;
            await DoScenarioButton.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.High, () => { });
            await LoggingScenario.DoScenarioAsync();
            DoScenarioButton.IsEnabled = true;           
        }
    }
}