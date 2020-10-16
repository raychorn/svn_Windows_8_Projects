//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using DiagnosticsHelper;
using HttpClientTransportHelper;
using SDKTemplate;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Windows.ApplicationModel.Background;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Graphics.Display;
using Windows.UI.Core;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace ControlChannelHttpClient
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario1 : SDKTemplate.Common.LayoutAwarePage, IDisposable
    {
        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;
        CoreDispatcher coreDispatcher;
        CommModule commModule;
        bool lockScreenAdded = false;
        TextBlock debugOutputTextBlock;
        private static Guid NetworkTaskRegistrationGuid = Guid.Empty;
        Uri serverUri;

        public Scenario1()
        {
            this.InitializeComponent();
            coreDispatcher = Diag.coreDispatcher = Window.Current.Dispatcher;
            ConnectButton.Visibility = Visibility.Collapsed;
        }

        public void Dispose()
        {
            if (commModule != null)
            {
                commModule.Dispose();
                commModule = null;
            }
        }        

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.  The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            debugOutputTextBlock = Diag.debugOutputTextBlock = DebugTextBlock;
        }

        #region Click Handlers

        private void ClientRole_Click(object sender, RoutedEventArgs e)
        {
            // To keep things simple, this button is disabled once clicked.
            Diag.DebugPrint("Client role selected");
            ClientRoleButton.IsEnabled = false;
            ClientInit();
        }
        #endregion

        async void ClientInit()
        {
            Diag.DebugPrint("Initializing client");

            commModule = new CommModule();

            // Lock screen is required to let in-process RealTimeCommunication related
            // background code to execute.
            if (!lockScreenAdded)
            {
                BackgroundAccessStatus status = await BackgroundExecutionManager.RequestAccessAsync();
                Diag.DebugPrint("Lock screen status: " + status);

                switch (status)
                {
                    case BackgroundAccessStatus.AllowedWithAlwaysOnRealTimeConnectivity:

                        // App is allowed to use RealTimeConnection  
                        // functionality even in low power mode.
                        lockScreenAdded = true;
                        break;
                    case BackgroundAccessStatus.AllowedMayUseActiveRealTimeConnectivity:

                        // App is allowed to use RealTimeConnection  
                        // functionality but not in low power mode.
                        lockScreenAdded = true;
                        break;
                    case BackgroundAccessStatus.Denied:

                        Diag.DebugPrint("Lock screen status was denied.");
                        break;
                }
            }

            if (lockScreenAdded)
            {
                // Now, enable the client settings.
                ClientSettings.Visibility = Visibility.Visible;
                ConnectButton.Visibility = Visibility.Visible;
            }

            Diag.DebugPrint("Initializing client done");
            return;
        }

        private enum ConnectionStates
        {
            NotConnected = 0,
            Connecting = 1,
            Connected = 2,
        };

        private static ConnectionStates connectionState = ConnectionStates.NotConnected;

        // Registers a background task with an network change system trigger.
        private void RegisterNetworkChangeTask()
        {
            try
            {
                if (NetworkTaskRegistrationGuid != Guid.Empty)
                {
                    IReadOnlyDictionary<Guid, IBackgroundTaskRegistration> allTasks = BackgroundTaskRegistration.AllTasks;
                    if (allTasks.ContainsKey(NetworkTaskRegistrationGuid))
                    {
                        Diag.DebugPrint("Network task is already registered.");
                        return;
                    }
                }

                BackgroundTaskBuilder myTaskBuilder = new BackgroundTaskBuilder();
                SystemTrigger myTrigger = new SystemTrigger(SystemTriggerType.NetworkStateChange, false);
                myTaskBuilder.SetTrigger(myTrigger);
                myTaskBuilder.TaskEntryPoint = "BackgroundTaskHelper.NetworkChangeTask";
                myTaskBuilder.Name = "Network change task";
                BackgroundTaskRegistration myTask = myTaskBuilder.Register();
                NetworkTaskRegistrationGuid = myTask.TaskId;
            }
            catch (Exception ex)
            {
                Diag.DebugPrint("Exception caught while setting up system event: " + ex.ToString());
            }
        }

        async private void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            if (connectionState == ConnectionStates.NotConnected)
            {
                if (!Uri.TryCreate(ServerUri.Text.Trim(), UriKind.Absolute, out serverUri))
                {
                    Diag.DebugPrint("Please provide a valid URI input.");
                    return;
                }

                ConnectButton.Content = "Connecting...";
                connectionState = ConnectionStates.Connecting;

                // Register for network status change notification
                RegisterNetworkChangeTask();

                // Finally, initiate the connection and set up transport
                // to be CCT capable. But do this heavy lifting outside of the UI thread.
                bool result = await Task<bool>.Factory.StartNew(() =>
                {
                    return commModule.SetupTransport(serverUri, GetType().Name);
                });
                Diag.DebugPrint("CommModule setup result: " + result);
                if (result == true)
                {
                    ConnectButton.Content = "Disconnect";
                    connectionState = ConnectionStates.Connected;
                }
                else
                {
                    ConnectButton.Content = "failed to connect. click to retry";
                    connectionState = ConnectionStates.NotConnected;
                }
            }
            else if (connectionState == ConnectionStates.Connected)
            {
                await Task.Factory.StartNew(() =>
                {
                    commModule.Reset();
                });

                connectionState = ConnectionStates.NotConnected;
                ConnectButton.Content = "Connect";
            }
        }

    }
}
