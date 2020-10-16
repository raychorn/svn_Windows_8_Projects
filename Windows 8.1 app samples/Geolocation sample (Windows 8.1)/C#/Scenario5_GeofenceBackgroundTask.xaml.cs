//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

using SDKTemplate;
using System;
using System.Threading;
using System.Threading.Tasks;
using Windows.Devices.Geolocation;
using Windows.Foundation;
using Windows.ApplicationModel.Background;
using Windows.Data.Json;
using Windows.Storage;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using Windows.UI.Notifications;

namespace Microsoft.Samples.Devices.Geolocation
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario5 : SDKTemplate.Common.LayoutAwarePage
    {
        public enum GeofenceReason
        {
            Add,
            Entered,
            Exited,
            Expired,
            Used
        };

        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;

        private IBackgroundTaskRegistration geofenceTask = null;
        private CancellationTokenSource cts = null;
        private ItemCollection geofenceBackgroundEvents = null;
        private Geolocator geolocator = null;
        private const long c_datetimeResolutionToSeconds = 10000000;    // conversion from 100 nano-second resolution to seconds
        private const int c_minEventArraySize = 10;		                // minimum size of Json array containing date/latitude/longitude + event data

        private class ItemCollection : System.Collections.ObjectModel.ObservableCollection<string>
        {
        }

        private const string SampleBackgroundTaskName = "SampleGeofenceBackgroundTask";
        private const string SampleBackgroundTaskEntryPoint = "BackgroundTask.GeofenceBackgroundTask";

        public Scenario5()
        {
            this.InitializeComponent();

            var settings = ApplicationData.Current.LocalSettings;
            if (settings.Values["GeofenceEvent"] != null)
            {
                settings.Values["GeofenceEvent"] = null;
            }

            // Get a geolocator object
            geolocator = new Geolocator();

            geofenceBackgroundEvents = new ItemCollection();

            // using data binding to the root page collection of GeofenceItems associated with events
            GeofenceEventsListView.DataContext = geofenceBackgroundEvents;
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached. The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            // Loop through all background tasks to see if SampleGeofenceBackgroundTask is already registered
            foreach (var cur in BackgroundTaskRegistration.AllTasks)
            {
                if (cur.Value.Name == SampleBackgroundTaskName)
                {
                    geofenceTask = cur.Value;
                    break;
                }
            }

            if (geofenceTask != null)
            {
                // Associate an event handler with the existing background task
                geofenceTask.Completed += new BackgroundTaskCompletedEventHandler(OnCompleted);

                BackgroundAccessStatus backgroundAccessStatus = BackgroundExecutionManager.GetAccessStatus();

                switch (backgroundAccessStatus)
                {
                    case BackgroundAccessStatus.Unspecified:
                    case BackgroundAccessStatus.Denied:
                        rootPage.NotifyUser("This application must be added to the lock screen before the background task will run.", NotifyType.ErrorMessage);
                        break;

                    default:
                        rootPage.NotifyUser("Background task is already registered. Waiting for next update...", NotifyType.ErrorMessage);
                        break;
                }

                RegisterBackgroundTaskButton.IsEnabled = false;
                UnregisterBackgroundTaskButton.IsEnabled = true;
            }
            else
            {
                RegisterBackgroundTaskButton.IsEnabled = true;
                UnregisterBackgroundTaskButton.IsEnabled = false;
            }
        }

        /// <summary>
        /// Invoked immediately before the Page is unloaded and is no longer the current source of a parent Frame.
        /// </summary>
        /// <param name="e">
        /// Event data that can be examined by overriding code. The event data is representative
        /// of the navigation that will unload the current Page unless canceled. The
        /// navigation can potentially be canceled by setting e.Cancel to true.
        /// </param>
        protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
        {
            // Just in case the original GetGeopositionAsync call is still active
            CancelGetGeoposition();

            if (geofenceTask != null)
            {
                // Remove the event handler
                geofenceTask.Completed -= new BackgroundTaskCompletedEventHandler(OnCompleted);
            }

            base.OnNavigatingFrom(e);
        }

        /// <summary>
        /// This is the click handler for the 'Register' button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        async private void RegisterBackgroundTask(object sender, RoutedEventArgs e)
        {
            // Get permission for a background task from the user. If the user has already answered once,
            // this does nothing and the user must manually update their preference via PC Settings.
            BackgroundAccessStatus backgroundAccessStatus = await BackgroundExecutionManager.RequestAccessAsync();

            // Regardless of the answer, register the background task. If the user later adds this application
            // to the lock screen, the background task will be ready to run.
            // Create a new background task builder
            BackgroundTaskBuilder geofenceTaskBuilder = new BackgroundTaskBuilder();

            geofenceTaskBuilder.Name = SampleBackgroundTaskName;
            geofenceTaskBuilder.TaskEntryPoint = SampleBackgroundTaskEntryPoint;

            // Create a new location trigger
            var trigger = new LocationTrigger(LocationTriggerType.Geofence);

            // Associate the locationi trigger with the background task builder
            geofenceTaskBuilder.SetTrigger(trigger);

            // If it is important that there is user presence and/or
            // internet connection when OnCompleted is called
            // the following could be called before calling Register()
            // SystemCondition condition = new SystemCondition(SystemConditionType.UserPresent | SystemConditionType.InternetAvailable);
            // geofenceTaskBuilder.AddCondition(condition);

            // Register the background task
            geofenceTask = geofenceTaskBuilder.Register();

            // Associate an event handler with the new background task
            geofenceTask.Completed += new BackgroundTaskCompletedEventHandler(OnCompleted);

            RegisterBackgroundTaskButton.IsEnabled = false;
            UnregisterBackgroundTaskButton.IsEnabled = true;

            switch (backgroundAccessStatus)
            {
            case BackgroundAccessStatus.Unspecified:
            case BackgroundAccessStatus.Denied:
                rootPage.NotifyUser("This application must be added to the lock screen before the background task will run.", NotifyType.ErrorMessage);
                break;

            default:
                // Ensure we have presented the location consent prompt (by asynchronously getting the current
                // position). This must be done here because the background task cannot display UI.
                GetGeopositionAsync();
                break;
            }
        }

        /// <summary>
        /// This is the click handler for the 'Unregister' button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void UnregisterBackgroundTask(object sender, RoutedEventArgs e)
        {
            // Remove the application from the lock screen
            BackgroundExecutionManager.RemoveAccess();

            // Unregister the background task
            if (null != geofenceTask)
            {
                geofenceTask.Unregister(true);
                geofenceTask = null;
            }

            rootPage.NotifyUser("Background task unregistered", NotifyType.StatusMessage);

            RegisterBackgroundTaskButton.IsEnabled = true;
            UnregisterBackgroundTaskButton.IsEnabled = false;
        }

        /// <summary>
        /// Helper method to invoke Geolocator.GetGeopositionAsync.
        /// </summary>
        async private void GetGeopositionAsync()
        {
            rootPage.NotifyUser("Checking permissions...", NotifyType.StatusMessage);

            try
            {
                // Get cancellation token
                cts = new CancellationTokenSource();
                CancellationToken token = cts.Token;

                // Carry out the operation
                Geoposition pos = await geolocator.GetGeopositionAsync().AsTask(token);

                // got permissions so clear the status string
                rootPage.NotifyUser("", NotifyType.StatusMessage);
            }
            catch (UnauthorizedAccessException)
            {
                rootPage.NotifyUser("Location Permissions disabled by user. Enable access through the settings charm to enable the background task.", NotifyType.StatusMessage);
            }
            catch (TaskCanceledException)
            {
                rootPage.NotifyUser("Permission check operation canceled.", NotifyType.StatusMessage);
            }
            catch (Exception ex)
            {
                // If there are no location sensors GetGeopositionAsync()
                // will timeout -- that is acceptable.
                const int WaitTimeoutHResult = unchecked((int)0x80070102);

                if (ex.HResult == WaitTimeoutHResult) // WAIT_TIMEOUT
                {
                    rootPage.NotifyUser("Operation accessing location sensors timed out. Possibly there are no location sensors.", NotifyType.StatusMessage);
                }
                else
                {
                    rootPage.NotifyUser(ex.ToString(), NotifyType.ErrorMessage);
                }
            }
            finally
            {
                cts = null;
            }
        }

        /// <summary>
        /// Helper method to invoke Geolocator.GetGeopositionAsync then update UI.
        /// </summary>
        async private void UpdateUIAfterPositionFix()
        {
            try
            {
                // Get cancellation token
                cts = new CancellationTokenSource();
                CancellationToken token = cts.Token;

                // Carry out the operation
                Geoposition pos = await geolocator.GetGeopositionAsync().AsTask(token);

                UpdateUIWithPosition(pos);
            }
            catch (UnauthorizedAccessException)
            {
                rootPage.NotifyUser("Location Permissions disabled by user. Enable access through the settings charm to enable the background task.", NotifyType.StatusMessage);
            }
            catch (TaskCanceledException)
            {
                rootPage.NotifyUser("Permission check operation canceled.", NotifyType.StatusMessage);
            }
            catch (Exception ex)
            {
                // If there are no location sensors GetGeopositionAsync()
                // will timeout -- that is acceptable.
                const int WaitTimeoutHResult = unchecked((int)0x80070102);

                if (ex.HResult == WaitTimeoutHResult) // WAIT_TIMEOUT
                {
                    rootPage.NotifyUser("Operation accessing location sensors timed out. Possibly there are no location sensors.", NotifyType.StatusMessage);
                }
                else
                {
                    rootPage.NotifyUser(ex.ToString(), NotifyType.ErrorMessage);
                }
            }
            finally
            {
                cts = null;
            }
        }

        private enum BackgroundData { Year = 0, Month, Day, Hour, Minute, Second, Period, Latitude, Longitude, FirstEventName };

        /// <summary>
        /// Updates the UI on the Background Geofencing page
        /// and showing how current position and time 
        /// can be used to filter out unwanted
        /// geofence events.
        /// </summary>
        private void UpdateUIWithPosition(Geoposition pos)
        { 
            // Update the UI with the completion status of the background task
            // The Run method of the background task sets the LocalSettings. 
            var settings = ApplicationData.Current.LocalSettings;

            // get status
            if (settings.Values["Status"] != null)
            {
                rootPage.NotifyUser(settings.Values["Status"].ToString(), NotifyType.StatusMessage);
            }

            // pop a toast for each geofence event
            // and add to listview
            if (settings.Values["GeofenceEvent"] != null)
            { 
                string geofenceEvent = settings.Values["GeofenceEvent"].ToString();

                JsonValue jsonValue = JsonValue.Parse(geofenceEvent);
                int arraySize = jsonValue.GetArray().Count;

                // the array contains
                // year
                // month
                // day
                // hour
                // minute
                // second
                // period (1:AM, 2:PM)
                // latitude
                // longitude

                // then for each geofence event
                // geofence.id
                // GeofenceReason (as double)

                // so minumum size of array should be 10
                if (arraySize >= c_minEventArraySize)
                {
                    Windows.Globalization.Calendar calendar = new Windows.Globalization.Calendar();
                    bool eventOfInterest = true;

                    calendar.Year = (int)jsonValue.GetArray().GetNumberAt((uint)BackgroundData.Year);
                    calendar.Month = (int)jsonValue.GetArray().GetNumberAt((uint)BackgroundData.Month);
                    calendar.Day = (int)jsonValue.GetArray().GetNumberAt((uint)BackgroundData.Day);
                    calendar.Hour = (int)jsonValue.GetArray().GetNumberAt((uint)BackgroundData.Hour);
                    calendar.Minute = (int)jsonValue.GetArray().GetNumberAt((uint)BackgroundData.Minute);
                    calendar.Second = (int)jsonValue.GetArray().GetNumberAt((uint)BackgroundData.Second);
                    calendar.Period = (int)jsonValue.GetArray().GetNumberAt((uint)BackgroundData.Period);

                    // NOTE TO DEVELOPER:
                    // This event can be filtered out if the
                    // geofence event location is stale.
                    DateTimeOffset eventDateTime = calendar.GetDateTime();

                    calendar.SetToNow();
                    DateTimeOffset nowDateTime = calendar.GetDateTime();

                    TimeSpan diffTimeSpan = nowDateTime - eventDateTime;

                    long deltaInSeconds = diffTimeSpan.Ticks / c_datetimeResolutionToSeconds;

                    // NOTE TO DEVELOPER:
                    // If the time difference between the geofence event and now is too large
                    // the eventOfInterest should be set to false.

                    string geofenceItemEvent = null;

                    int numEventsOfInterest = (int)(arraySize - BackgroundData.FirstEventName) / 2;

                    if (eventOfInterest)
                    {
                        double latitudeEvent = jsonValue.GetArray().GetNumberAt((uint)BackgroundData.Latitude);
                        double longitudeEvent = jsonValue.GetArray().GetNumberAt((uint)BackgroundData.Longitude);

                        // NOTE TO DEVELOPER:
                        // This event can be filtered out if the
                        // geofence event location is too far away.
                        if ((latitudeEvent != pos.Coordinate.Point.Position.Latitude) ||
                            (longitudeEvent != pos.Coordinate.Point.Position.Longitude))
                        {
                            // NOTE TO DEVELOPER:
                            // Use an algorithm like Haversine to determine
                            // the distance between the current location (pos.Coordinate)
                            // and the location of the geofence event (latitudeEvent/longitudeEvent).
                            // If too far apart set eventOfInterest to false to
                            // filter the event out.
                        }

                        if (eventOfInterest)
                        {
                            for (uint loop = (uint)BackgroundData.FirstEventName; loop < arraySize; )
                            {
                                string itemId = jsonValue.GetArray().GetStringAt(loop++);
                                GeofenceReason reason = (GeofenceReason)jsonValue.GetArray().GetNumberAt(loop++);

                                geofenceItemEvent = new string(itemId.ToCharArray());

                                switch (reason)
                                {
                                case GeofenceReason.Add:
                                    geofenceItemEvent += " (Added)";
                                    break;

                                case GeofenceReason.Entered:
                                    geofenceItemEvent += " (Entered)";
                                    break;

                                case GeofenceReason.Exited:
                                    geofenceItemEvent += " (Exited)";
                                    break;

                                case GeofenceReason.Expired:
                                    geofenceItemEvent += " (Removed/Expired)";
                                    break;

                                case GeofenceReason.Used:
                                    geofenceItemEvent += " (Removed/Used)";
                                    break;

                                default:
                                    break;
                                }

                                if (0 != geofenceItemEvent.Length)
                                {
                                    // now add event to listview
                                    geofenceBackgroundEvents.Insert(0, geofenceItemEvent);
                                }
                                else
                                {
                                    --numEventsOfInterest;
                                }
                            }
                        }
                    }

                    if (settings.Values["GeofenceEvent"] != null)
                    {
                        settings.Values["GeofenceEvent"] = null;
                    }

                    if (eventOfInterest && (0 != numEventsOfInterest))
                    {
                        DoToast(numEventsOfInterest, geofenceItemEvent);
                        DoTile(numEventsOfInterest, geofenceItemEvent);
                        DoBadge(numEventsOfInterest);
                    }
                }
            }
        }

        /// <summary>
        /// Helper method to pop a toast
        /// </summary>
        private void DoToast(int numEventsOfInterest, string eventName)
        {
            // pop a toast for each geofence event
            ToastNotifier ToastNotifier = ToastNotificationManager.CreateToastNotifier();

            // Create a two line toast and add audio reminder

            // Here the xml that will be passed to the 
            // ToastNotification for the toast is retrieved
            Windows.Data.Xml.Dom.XmlDocument toastXml = ToastNotificationManager.GetTemplateContent(ToastTemplateType.ToastText02);

            // Set both lines of text
            Windows.Data.Xml.Dom.XmlNodeList toastNodeList = toastXml.GetElementsByTagName("text");
            toastNodeList.Item(0).AppendChild(toastXml.CreateTextNode("Geolocation Sample"));

            if (1 == numEventsOfInterest)
            {
                toastNodeList.Item(1).AppendChild(toastXml.CreateTextNode(eventName));
            }
            else
            {
                string secondLine = "There are " + numEventsOfInterest + " new geofence events";
                toastNodeList.Item(1).AppendChild(toastXml.CreateTextNode(secondLine));
            }

            // now create a xml node for the audio source
            Windows.Data.Xml.Dom.IXmlNode toastNode = toastXml.SelectSingleNode("/toast");
            Windows.Data.Xml.Dom.XmlElement audio = toastXml.CreateElement("audio");
            audio.SetAttribute("src", "ms-winsoundevent:Notification.SMS");

            ToastNotification toast = new ToastNotification(toastXml);
            ToastNotifier.Show(toast);
        }

        /// <summary>
        /// Helper method to send notification text to the tile
        /// </summary>
        private void DoTile(int numEventsOfInterest, string eventName)
        {
            // update tile
            TileUpdater tileUpdater = TileUpdateManager.CreateTileUpdaterForApplication();
            tileUpdater.EnableNotificationQueue(true);

            tileUpdater.Clear();

            Windows.Data.Xml.Dom.XmlDocument tileXml = TileUpdateManager.GetTemplateContent(TileTemplateType.TileSquare150x150Text02);

            Windows.Data.Xml.Dom.XmlNodeList tileNodeList = tileXml.GetElementsByTagName("text");
            tileNodeList.Item(0).AppendChild(tileXml.CreateTextNode("Geolocation Sample"));

            if (1 == numEventsOfInterest)
            {
                tileNodeList.Item(1).AppendChild(tileXml.CreateTextNode(eventName));
            }
            else
            {
                string secondLine = "There are " + numEventsOfInterest + " new geofence events";
                tileNodeList.Item(1).AppendChild(tileXml.CreateTextNode(secondLine));
            }

            TileNotification tile = new TileNotification(tileXml);
            tileUpdater.Update(tile);
        }

        /// <summary>
        /// Helper method to update badge with number of events
        /// </summary>
        private void DoBadge(int numEventsOfInterest)
        {
            BadgeUpdater badgeUpdater = BadgeUpdateManager.CreateBadgeUpdaterForApplication();

            string badgeXmlString = "<badge value='" + numEventsOfInterest + "'/>"; ;
            Windows.Data.Xml.Dom.XmlDocument badgeXml = new Windows.Data.Xml.Dom.XmlDocument();
            badgeXml.LoadXml(badgeXmlString);

            BadgeNotification badge = new BadgeNotification(badgeXml);
            badgeUpdater.Update(badge);
        }

        /// <summary>
        /// Helper method to cancel the GetGeopositionAsync request (if any).
        /// </summary>
        private void CancelGetGeoposition()
        {
            if (cts != null)
            {
                cts.Cancel();
                cts = null;
            }
        }

        /// <summary>
        /// This is the callback when background event has been handled
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        async private void OnCompleted(IBackgroundTaskRegistration sender, BackgroundTaskCompletedEventArgs e)
        {
            if (sender != null)
            {
                // Update the UI with progress reported by the background task
                await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                {
                    try
                    {
                        // If the background task threw an exception, display the exception in
                        // the error text box.
                        e.CheckResult();

                        // This method waits for the current
                        // position to be acquired for comparison
                        // to the latitude & longitude last tracked 
                        // by the GeofenceMonitor when the background
                        // event took place.
                        //
                        // If comparison of location isn't important
                        // the code in UpdateUIWithPosition() could
                        // be placed here.
                        //
                        // Also, if it were important that there 
                        // be user presence and/or internet
                        // connection when OnComplete is called
                        // the background task can be 
                        // registered with those conditions.
                        // See note in RegisterBackgroundTask()
                        // on how to do this.
                        UpdateUIAfterPositionFix();
                    }
                    catch (Exception ex)
                    {
                        // The background task had an error
                        rootPage.NotifyUser(ex.ToString(), NotifyType.ErrorMessage);
                    }
                });
            }
        }
    }
}

