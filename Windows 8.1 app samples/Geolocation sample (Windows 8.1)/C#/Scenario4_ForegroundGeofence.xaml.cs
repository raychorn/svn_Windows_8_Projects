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
using System.Collections.Generic;
using Windows.Devices.Enumeration;
using Windows.Devices.Geolocation;
using Windows.Devices.Geolocation.Geofencing;
using Windows.Foundation;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace Microsoft.Samples.Devices.Geolocation
{
    // GeofenceItem implements IEquatable to allow
    // removal of objects in the collection
    public class GeofenceItem : IEquatable<GeofenceItem>
    {
        private Geofence geofence;
        private string id;

        public GeofenceItem(Geofence geofence)
        {
            this.geofence = geofence;
            this.id = geofence.Id;
        }

        public bool Equals(GeofenceItem other)
        {
            bool isEqual = false;
            if (Id == other.Id)
            {
                isEqual = true;
            }

            return isEqual;
        }

        public Windows.Devices.Geolocation.Geofencing.Geofence Geofence
        {
            get
            {
                return geofence;
            }
        }

        public string Id
        {
            get
            {
                return id;
            }
        }

        public double Latitude
        {
            get
            {
                Geocircle circle = geofence.Geoshape as Geocircle;
                return circle.Center.Latitude;
            }
        }

        public double Longitude
        {
            get
            {
                Geocircle circle = geofence.Geoshape as Geocircle;
                return circle.Center.Longitude;
            }
        }

        public double Radius
        {
            get
            {
                Geocircle circle = geofence.Geoshape as Geocircle;
                return circle.Radius;
            }
        }

        public bool SingleUse
        {
            get
            {
                return geofence.SingleUse;
            }
        }

        public MonitoredGeofenceStates MonitoredStates
        {
            get
            {
                return geofence.MonitoredStates;
            }
        }

        public TimeSpan Dwell
        {
            get
            {
                return geofence.DwellTime;
            }
        }

        public DateTimeOffset Start
        {
            get
            {
                return geofence.StartTime;
            }
        }

        public TimeSpan Duration
        {
            get
            {
                return geofence.Duration;
            }
        }
    }

    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario4 : SDKTemplate.Common.LayoutAwarePage
    {
        private bool latitudeSet = false;
        private bool longitudeSet = false;
        private bool radiusSet = false;
        private bool permissionsChecked = false;
        private bool inGetPositionAsync = false;
        private bool geofenceStateChangedRegistered = false;
        private CancellationTokenSource cts = null;
        private Geolocator geolocator = null;
        private GeofenceItemCollection geofenceCollection = null;
        private GeofenceEventItemCollection geofenceEvents = null;
        private DeviceAccessInformation accessInfo;

        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;

        public Scenario4()
        {
            this.InitializeComponent();

            try
            {
                geofenceCollection = new GeofenceItemCollection();
                geofenceEvents = new GeofenceEventItemCollection();

                // Get a geolocator object
                geolocator = new Geolocator();

                // using data binding to the root page collection of GeofenceItems
                GeofenceRegisteredListView.DataContext = geofenceCollection;

                // using data binding to the root page collection of GeofenceItems associated with events
                GeofenceEventsListView.DataContext = geofenceEvents;

                FillRegisteredGeofenceListViewWithExistingGeofences();

                accessInfo = DeviceAccessInformation.CreateFromDeviceClass(DeviceClass.Location);
                accessInfo.AccessChanged += OnAccessChangedHandler;

                // register for state change events
                GeofenceMonitor.Current.GeofenceStateChanged += OnGeofenceStateChangedHandler;
                GeofenceMonitor.Current.StatusChanged += OnStatusChangedHandler;

                geofenceStateChangedRegistered = true;
            }
            catch (UnauthorizedAccessException)
            {
                if (DeviceAccessStatus.DeniedByUser == accessInfo.CurrentStatus)
                {
                    rootPage.NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType.StatusMessage);
                }
                else if (DeviceAccessStatus.DeniedBySystem == accessInfo.CurrentStatus)
                {
                    rootPage.NotifyUser("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", NotifyType.StatusMessage);
                }
                else if (DeviceAccessStatus.Unspecified == accessInfo.CurrentStatus)
                {
                    rootPage.NotifyUser("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", NotifyType.StatusMessage);
                }
            }
            catch (Exception ex)
            {
                // GeofenceMonitor failed in adding a geofence
                // exceptions could be from out of memory, lat/long out of range,
                // too long a name, not a unique name, specifying an activation
                // time + duration that is still in the past
                rootPage.NotifyUser(ex.ToString(), NotifyType.ErrorMessage);
            }
        }

        void FillRegisteredGeofenceListViewWithExistingGeofences()
        {
            var geofences = GeofenceMonitor.Current.Geofences;

            foreach (Geofence geofence in geofences)
            {
                AddGeofenceToRegisteredGeofenceListView(geofence);
            }
        }

        void GeofenceDurationSecondTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Seconds", GeofenceDurationSecond);
        }

        void GeofenceDurationMinuteTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Minute", GeofenceDurationMinute);
        }

        void GeofenceDurationHourTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Hour", GeofenceDurationHour);
        }

        void GeofenceDurationDayTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Day", GeofenceDurationDay);
        }

        void GeofenceDwellTimeSecondTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Seconds", GeofenceDwellTimeSecond);
        }

        void GeofenceDwellTimeMinuteTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Minute", GeofenceDwellTimeMinute);
        }

        void GeofenceDwellTimeHourTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Hour", GeofenceDwellTimeHour);
        }

        void GeofenceDwellTimeDayTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Day", GeofenceDwellTimeDay);
        }

        void GeofenceRadiusTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            radiusSet = TextChangedHandlerDouble(false, "Radius", GeofenceRadius);

            DetermineCreateGeofenceButtonEnableState();
        }

        void GeofenceLongitudeTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            longitudeSet = TextChangedHandlerDouble(false, "Longitude", GeofenceLongitude);

            DetermineCreateGeofenceButtonEnableState();
        }

        void GeofenceLatitudeTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            latitudeSet = TextChangedHandlerDouble(false, "Latitude", GeofenceLatitude);

            DetermineCreateGeofenceButtonEnableState();
        }

        void GeofenceNameTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            // get number of characters
            int charCount = GeofenceName.Text.Length;

            CharCount.Text = charCount.ToString() + " characters";
        }

        void GeofenceStartTimeSecondTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Seconds", GeofenceStartTimeSecond);
        }

        void GeofenceStartTimeMinuteTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Minute", GeofenceStartTimeMinute);
        }

        void GeofenceStartTimeHourTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Hour", GeofenceStartTimeHour);
        }

        void GeofenceStartTimeDayTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Day", GeofenceStartTimeDay);
        }

        void GeofenceStartTimeMonthTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Month", GeofenceStartTimeMonth);
        }

        void GeofenceStartTimeYearTextChangedHandler(object sender, TextChangedEventArgs e)
        {
            TextChangedHandlerInt(true, "Year", GeofenceStartTimeYear);
        }

        void GeofenceRegisteredListViewSelectionChangedHandler(object sender, SelectionChangedEventArgs e)
        {
            IList<object> list = e.AddedItems;

            if (0 == list.Count)
            {
                // disable the remove button
                RemoveGeofenceItem.IsEnabled = false;
            }
            else
            {
                // enable the remove button
                RemoveGeofenceItem.IsEnabled = true;

                // update controls with the values from this geofence item
                // get selected item
                GeofenceItem item = GeofenceRegisteredListView.SelectedItem as GeofenceItem;

                RefreshControlsFromGeofenceItem(item);

                DetermineCreateGeofenceButtonEnableState();

            }
        }

        private void RefreshControlsFromGeofenceItem(GeofenceItem item)
        {
            if (null != item)
            {
                GeofenceName.Text = item.Id;
                GeofenceLatitude.Text = item.Latitude.ToString();
                GeofenceLongitude.Text = item.Longitude.ToString();
                GeofenceRadius.Text = item.Radius.ToString();

                GeofenceSingleUse.IsChecked = item.SingleUse;

                MonitoredGeofenceStates states = item.MonitoredStates;
                GeofenceEnter.IsChecked = states.HasFlag(MonitoredGeofenceStates.Entered);
                GeofenceExit.IsChecked = states.HasFlag(MonitoredGeofenceStates.Exited);
                GeofenceRemove.IsChecked = states.HasFlag(MonitoredGeofenceStates.Removed);

                if (0 != item.Dwell.Days)
                {
                    GeofenceDwellTimeDay.Text = item.Dwell.Days.ToString();
                }
                else
                {
                    GeofenceDwellTimeDay.Text = "";
                }
                if (0 != item.Dwell.Hours)
                {
                    GeofenceDwellTimeHour.Text = item.Dwell.Hours.ToString();
                }
                else
                {
                    GeofenceDwellTimeHour.Text = "";
                }
                if (0 != item.Dwell.Minutes)
                {
                    GeofenceDwellTimeMinute.Text = item.Dwell.Minutes.ToString();
                }
                else
                {
                    GeofenceDwellTimeMinute.Text = "";
                }
                if (0 != item.Dwell.Seconds)
                {
                    GeofenceDwellTimeSecond.Text = item.Dwell.Seconds.ToString();
                }
                else
                {
                    GeofenceDwellTimeSecond.Text = "";
                }

                if (0 != item.Duration.Days)
                {
                    GeofenceDurationDay.Text = item.Duration.Days.ToString();
                }
                else
                {
                    GeofenceDurationDay.Text = "";
                }
                if (0 != item.Duration.Hours)
                {
                    GeofenceDurationHour.Text = item.Duration.Hours.ToString();
                }
                else
                {
                    GeofenceDurationHour.Text = "";
                }
                if (0 != item.Duration.Minutes)
                {
                    GeofenceDurationMinute.Text = item.Duration.Minutes.ToString();
                }
                else
                {
                    GeofenceDurationMinute.Text = "";
                }
                if (0 != item.Duration.Seconds)
                {
                    GeofenceDurationSecond.Text = item.Duration.Seconds.ToString();
                }
                else
                {
                    GeofenceDurationSecond.Text = "";
                }

                if (0 != item.Start.Year)
                {
                    GeofenceStartTimeYear.Text = item.Start.Year.ToString();
                }
                else
                {
                    GeofenceStartTimeYear.Text = "";
                }
                if (0 != item.Start.Month)
                {
                    GeofenceStartTimeMonth.Text = item.Start.Month.ToString();
                }
                else
                {
                    GeofenceStartTimeMonth.Text = "";
                }
                if (0 != item.Start.Day)
                {
                    GeofenceStartTimeDay.Text = item.Start.Day.ToString();
                }
                else
                {
                    GeofenceStartTimeDay.Text = "";
                }
                if (0 != item.Start.Hour)
                {
                    GeofenceStartTimeHour.Text = item.Start.Hour.ToString();
                }
                else
                {
                    GeofenceStartTimeHour.Text = "";
                }
                if (0 != item.Start.Minute)
                {
                    GeofenceStartTimeMinute.Text = item.Start.Minute.ToString();
                }
                else
                {
                    GeofenceStartTimeMinute.Text = "";
                }
                if (0 != item.Start.Second)
                {
                    GeofenceStartTimeSecond.Text = item.Start.Second.ToString();
                }
                else
                {
                    GeofenceStartTimeSecond.Text = "";
                }

                // Update flags used to enable Create Geofence button
                GeofenceNameTextChangedHandler(null, null);
                GeofenceLongitudeTextChangedHandler(null, null);
                GeofenceLatitudeTextChangedHandler(null, null);
                GeofenceRadiusTextChangedHandler(null, null);
            }
        }

        private bool TextChangedHandlerDouble(bool nullAllowed, string name, TextBox e)
        {
            bool valueSet = false;

            try
            {
                double value = Double.Parse(e.Text);

                valueSet = true;

                // clear out status message
                rootPage.NotifyUser("", NotifyType.StatusMessage);
            }
            catch (ArgumentNullException)
            {
                if (false == nullAllowed)
                {
                    if (null != name)
                    {
                        rootPage.NotifyUser(name + " needs a value", NotifyType.StatusMessage);
                    }
                }
                else
                {
                    valueSet = true;
                }
            }
            catch (FormatException)
            {
                if (null != name)
                {
                    rootPage.NotifyUser(name + " must be a number", NotifyType.StatusMessage);
                }
            }
            catch (OverflowException)
            {
                if (null != name)
                {
                    rootPage.NotifyUser(name + " is out of bounds", NotifyType.StatusMessage);
                }
            }

            return valueSet;
        }

        private bool TextChangedHandlerInt(bool nullAllowed, string name, TextBox e)
        {
            bool valueSet = false;

            try
            {
                int value = int.Parse(e.Text);

                valueSet = true;

                // clear out status message
                rootPage.NotifyUser("", NotifyType.StatusMessage);
            }
            catch (ArgumentNullException)
            {
                if (false == nullAllowed)
                {
                    if (null != name)
                    {
                        rootPage.NotifyUser(name + " needs a value", NotifyType.StatusMessage);
                    }
                }
                else
                {
                    valueSet = true;
                }
            }
            catch (FormatException)
            {
                if (null != name)
                {
                    rootPage.NotifyUser(name + " must be a number", NotifyType.StatusMessage);
                }
            }
            catch (OverflowException)
            {
                if (null != name)
                {
                    rootPage.NotifyUser(name + " is out of bounds", NotifyType.StatusMessage);
                }
            }

            return valueSet;
        }

        private string GetTimeStampedMessage(string eventCalled)
        {
            string message;

            Windows.Globalization.DateTimeFormatting.DateTimeFormatter formatter = new Windows.Globalization.DateTimeFormatting.DateTimeFormatter("longtime");
            Windows.Globalization.Calendar calendar = new Windows.Globalization.Calendar();
            calendar.SetToNow();

            message = eventCalled + " " + formatter.Format(calendar.GetDateTime());

            return message;
        }

        public async void OnAccessChangedHandler(
            DeviceAccessInformation sender,
            DeviceAccessChangedEventArgs args
            )
        { 
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                string eventDescription = GetTimeStampedMessage("Device Access Status");

                if (DeviceAccessStatus.DeniedByUser == args.Status)
                {
                    eventDescription += " (DeniedByUser)";

                    geofenceEvents.Insert(0, eventDescription);

                    rootPage.NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType.StatusMessage);
                }
                else if (DeviceAccessStatus.DeniedBySystem == args.Status)
                {
                    eventDescription += " (DeniedBySystem)";

                    geofenceEvents.Insert(0, eventDescription);

                    rootPage.NotifyUser("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", NotifyType.StatusMessage);
                }
                else if (DeviceAccessStatus.Unspecified == args.Status)
                {
                    eventDescription += " (Unspecified)";

                    geofenceEvents.Insert(0, eventDescription);

                    rootPage.NotifyUser("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", NotifyType.StatusMessage);
                }
                else if (DeviceAccessStatus.Allowed == args.Status)
                {
                    eventDescription += " (Allowed)";

                    geofenceEvents.Insert(0, eventDescription);

                    // clear status
                    rootPage.NotifyUser("", NotifyType.StatusMessage);

                    if (false == geofenceStateChangedRegistered)
                    {
                        // register for state change events
                        GeofenceMonitor.Current.GeofenceStateChanged += OnGeofenceStateChangedHandler;
                        GeofenceMonitor.Current.StatusChanged += OnStatusChangedHandler;

                        geofenceStateChangedRegistered = true;
                    }
                }
                else
                {
                    rootPage.NotifyUser("Unknown device access information status", NotifyType.StatusMessage);
                }
            });
        }

        public async void OnStatusChangedHandler(
                    GeofenceMonitor sender,
                    object e
                    )
        {
            await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
            {
                var status = sender.Status;

                string eventDescription = GetTimeStampedMessage("Geofence Status Changed");

                if (GeofenceMonitorStatus.Ready == status)
                {
                    eventDescription += " (Ready)";

                    geofenceEvents.Insert(0, eventDescription);
                }
                else if (GeofenceMonitorStatus.Initializing == status)
                {
                    eventDescription += " (Initializing)";

                    geofenceEvents.Insert(0, eventDescription);
                }
                else if (GeofenceMonitorStatus.NoData == status)
                {
                    eventDescription += " (NoData)";

                    geofenceEvents.Insert(0, eventDescription);
                }
                else if (GeofenceMonitorStatus.Disabled == status)
                {
                    eventDescription += " (Disabled)";

                    geofenceEvents.Insert(0, eventDescription);
                }
                else if (GeofenceMonitorStatus.NotInitialized == status)
                {
                    eventDescription += " (NotInitialized)";

                    geofenceEvents.Insert(0, eventDescription);
                }
                else if (GeofenceMonitorStatus.NotAvailable == status)
                {
                    eventDescription += " (NotAvailable)";

                    geofenceEvents.Insert(0, eventDescription);
                }
            });
        }

        public async void OnGeofenceStateChangedHandler(
            GeofenceMonitor sender,
            object e
            )
        {
            var reports = sender.ReadReports();

            foreach (GeofenceStateChangeReport report in reports)
            {
                GeofenceState state = report.NewState;

                Geofence geofence = report.Geofence;
                string eventDescription = GetTimeStampedMessage(geofence.Id);

                if (state == GeofenceState.Removed)
                {
                    GeofenceRemovalReason reason = report.RemovalReason;

                    if (reason == GeofenceRemovalReason.Expired)
                    {
                        await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                        {
                            eventDescription += " (Removed/Expired)";

                            geofenceEvents.Insert(0, eventDescription);
                        });
                    }
                    else if (reason == GeofenceRemovalReason.Used)
                    {
                        await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                        {
                            eventDescription += " (Removed/Used)";

                            geofenceEvents.Insert(0, eventDescription);
                        });
                    }

                    await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                    {
                        GeofenceItem itemToRemove = new GeofenceItem(geofence);

                        // remove this item from the collection
                        geofenceCollection.Remove(itemToRemove);
                    });
                }
                else if (state == GeofenceState.Entered)
                {
                    await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                    {
                        eventDescription += " (Entered)";

                        geofenceEvents.Insert(0, eventDescription);
                    });
                }
                else if (state == GeofenceState.Exited)
                {
                    //await Dispatcher.RunAsync(CoreDispatcherPriority.Normal,
                    //async () =>
                    await Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
                    {
                        eventDescription += " (Exited)";

                        geofenceEvents.Insert(0, eventDescription);
                    });
                }
            }
        }

        // are settings available so a geofence can be created?
        private bool SettingsAvailable()
        {
            bool fSettingsAvailable = false;

            if ((true == latitudeSet) &&
                (true == longitudeSet) &&
                (true == radiusSet))
            {
                // also need to test if data is good
                fSettingsAvailable = true;
            }

            return fSettingsAvailable;
        }

        private void DetermineCreateGeofenceButtonEnableState()
        {
            CreateGeofenceButton.IsEnabled = SettingsAvailable();
        }

        // add geofence to listview
        private void AddGeofenceToRegisteredGeofenceListView(Geofence geofence)
        {
            GeofenceItem item = new GeofenceItem(geofence);

            // the registered geofence listview is data bound
            // to the collection stored in the root page
            geofenceCollection.Insert(0, item);
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached. The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
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
            if (true == inGetPositionAsync)
            {
                if (cts != null)
                {
                    cts.Cancel();
                    cts = null;
                }
            }

            GeofenceMonitor.Current.GeofenceStateChanged -= OnGeofenceStateChangedHandler;
            GeofenceMonitor.Current.StatusChanged -= OnStatusChangedHandler;

            base.OnNavigatingFrom(e);
        }

        /// <summary>
        /// This is the click handler for the 'Remove Geofence Item' button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Remove(object sender, RoutedEventArgs e)
        {
            try
            {
                if (null != GeofenceRegisteredListView.SelectedItem)
                {
                    // get selected item
                    GeofenceItem itemToRemove = GeofenceRegisteredListView.SelectedItem as GeofenceItem;

                    // get array of geofences from the GeofenceMonitor
                    var geofences = GeofenceMonitor.Current.Geofences;

                    var geofence = itemToRemove.Geofence;

                    var index = geofences.IndexOf(geofence);

                    if (-1 != index)
                    {
                        geofences.RemoveAt(index);
                    }
                    else
                    {
                        string strMsg = "Could not find GeofenceItem " + itemToRemove.Id + " in GeofenceMonitor";

                        rootPage.NotifyUser(strMsg, NotifyType.StatusMessage);
                    }

                    // remove this item from the registered geofence listview collection
                    if (false == geofenceCollection.Remove(itemToRemove))
                    {
                        string strMsg = "Could not find GeofenceItem " + itemToRemove.Id + " in geofenceCollection";

                        rootPage.NotifyUser(strMsg, NotifyType.StatusMessage);
                    }
                }
            }
            catch (Exception ex)
            {
                rootPage.NotifyUser(ex.ToString(), NotifyType.ErrorMessage);
            }
        }

        private Geofence GenerateGeofence()
        {
            Geofence geofence = null;

            try
            {
                string fenceKey = new string(GeofenceName.Text.ToCharArray());

                BasicGeoposition position;
                position.Latitude = Double.Parse(GeofenceLatitude.Text);
                position.Longitude = Double.Parse(GeofenceLongitude.Text);
                position.Altitude = 0.0;
                double radius = Double.Parse(GeofenceRadius.Text);

                // the geofence is a circular region
                Geocircle geocircle = new Geocircle(position, radius);

                bool singleUse = (bool)GeofenceSingleUse.IsChecked;

                // want to listen for enter geofence, exit geofence and remove geofence events
                // you can select a subset of these event states
                MonitoredGeofenceStates mask = 0;

                if (true == GeofenceEnter.IsChecked)
                {
                    mask |= MonitoredGeofenceStates.Entered;
                }
                if (true == GeofenceExit.IsChecked)
                {
                    mask |= MonitoredGeofenceStates.Exited;
                }
                if (true == GeofenceRemove.IsChecked)
                {
                    mask |= MonitoredGeofenceStates.Removed;
                }

                // setting up how long you need to be in geofence for enter event to fire
                int dwellTimeDays = 0;
                int dwellTimeHours = 0;
                int dwellTimeMinutes = 0;
                int dwellTimeSeconds = 0;

                bool useDwellTime = false;

                // use dwell if at least one textbox has text
                if ((null != GeofenceDwellTimeDay.Text) ||
                    (null != GeofenceDwellTimeHour.Text) ||
                    (null != GeofenceDwellTimeMinute.Text) ||
                    (null != GeofenceDwellTimeSecond.Text))
                {
                    if (true == TextChangedHandlerInt(true, null, GeofenceDwellTimeDay))
                    {
                        dwellTimeDays = int.Parse(GeofenceDwellTimeDay.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceDwellTimeHour))
                    {
                        dwellTimeHours = int.Parse(GeofenceDwellTimeHour.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceDwellTimeMinute))
                    {
                        dwellTimeMinutes = int.Parse(GeofenceDwellTimeMinute.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceDwellTimeSecond))
                    {
                        dwellTimeSeconds = int.Parse(GeofenceDwellTimeSecond.Text);
                    }
                }

                if ((0 != dwellTimeDays) || (0 != dwellTimeHours) || (0 != dwellTimeMinutes) || (0 != dwellTimeSeconds))
                {
                    useDwellTime = true;
                }

                // setting up how long the geofence should be active
                int durationDays = 0;
                int durationHours = 0;
                int durationMinutes = 0;
                int durationSeconds = 0;

                bool useDuration = false;

                // use duration if at least one textbox has text
                if ((null != GeofenceDurationDay.Text) ||
                    (null != GeofenceDurationHour.Text) ||
                    (null != GeofenceDurationMinute.Text) ||
                    (null != GeofenceDurationSecond.Text))
                {
                    if (true == TextChangedHandlerInt(true, null, GeofenceDurationDay))
                    {
                        durationDays = int.Parse(GeofenceDurationDay.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceDurationHour))
                    {
                        durationHours = int.Parse(GeofenceDurationHour.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceDurationMinute))
                    {
                        durationMinutes = int.Parse(GeofenceDurationMinute.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceDurationSecond))
                    {
                        durationSeconds = int.Parse(GeofenceDurationSecond.Text);
                    }
                }

                if ((0 != durationDays) || (0 != durationHours) || (0 != durationMinutes) || (0 != durationSeconds))
                {
                    useDuration = true;
                }

                // setting up the start time of the geofence
                int startTimeYear = 0;
                int startTimeMonth = 0;
                int startTimeDay = 0;
                int startTimeHour = 0;
                int startTimeMinute = 0;
                int startTimeSecond = 0;

                bool useStartTime = false;

                // use duration if at least one textbox has text
                if ((null != GeofenceStartTimeYear.Text) ||
                    (null != GeofenceStartTimeMonth.Text) ||
                    (null != GeofenceStartTimeDay.Text) ||
                    (null != GeofenceStartTimeHour.Text) ||
                    (null != GeofenceStartTimeMinute.Text) ||
                    (null != GeofenceStartTimeSecond.Text))
                {
                    if (true == TextChangedHandlerInt(true, null, GeofenceStartTimeYear))
                    {
                        startTimeYear = int.Parse(GeofenceStartTimeYear.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceStartTimeMonth))
                    {
                        startTimeMonth = int.Parse(GeofenceStartTimeMonth.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceStartTimeDay))
                    {
                        startTimeDay = int.Parse(GeofenceStartTimeDay.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceStartTimeHour))
                    {
                        startTimeHour = int.Parse(GeofenceStartTimeHour.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceStartTimeMinute))
                    {
                        startTimeMinute = int.Parse(GeofenceStartTimeMinute.Text);
                    }
                    if (true == TextChangedHandlerInt(true, null, GeofenceStartTimeSecond))
                    {
                        startTimeSecond = int.Parse(GeofenceStartTimeSecond.Text);
                    }
                }

                if ((0 != startTimeYear) || (0 != startTimeMonth) || (0 != startTimeDay) || (0 != startTimeHour) || (0 != startTimeMinute) || (0 != startTimeSecond))
                {
                    useStartTime = true;
                }

                if ((false == useStartTime) && (false == useDuration))
                {
                    if (true == useDwellTime)
                    {
                        // mask must be set, so set it to default values
                        if (0 == mask)
                        {
                            mask = MonitoredGeofenceStates.Entered | MonitoredGeofenceStates.Exited;
                        }

                        TimeSpan dwellTime = new TimeSpan(dwellTimeDays, dwellTimeHours, dwellTimeMinutes, dwellTimeSeconds);
                        geofence = new Geofence(fenceKey, geocircle, mask, singleUse, dwellTime);
                    }
                    else
                    {
                        if (0 != mask)
                        {
                            geofence = new Geofence(fenceKey, geocircle, mask, singleUse);
                        }
                        else
                        {
                            geofence = new Geofence(fenceKey, geocircle);
                        }
                    }
                }
                else
                {
                    // mask must be set, so set it to default values
                    if (0 == mask)
                    {
                        mask = MonitoredGeofenceStates.Entered | MonitoredGeofenceStates.Exited;
                    }

                    TimeSpan dwellTime = new TimeSpan(dwellTimeDays, dwellTimeHours, dwellTimeMinutes, dwellTimeSeconds);
                    TimeSpan duration = new TimeSpan(durationDays, durationHours, durationMinutes, durationSeconds);
                    DateTime startDateTime = new DateTime(startTimeYear, startTimeMonth, startTimeDay, startTimeHour, startTimeMinute, startTimeSecond);
                    DateTimeOffset startTime = new DateTimeOffset(startDateTime);

                    geofence = new Geofence(fenceKey, geocircle, mask, singleUse, dwellTime, startTime, duration);
                }
            }
            catch (Exception ex)
            {
                // GeofenceMonitor failed in adding a geofence
                // exceptions could be from out of memory, lat/long out of range,
                // too long a name, not a unique name, specifying an activation
                // time + duration that is still in the past
                rootPage.NotifyUser(ex.ToString(), NotifyType.ErrorMessage);
            }

            return geofence;
        }

        /// <summary>
        /// This is the click handler for the 'Create Geofence' button.
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void CreateGeofence(object sender, RoutedEventArgs e)
        {
            try
            {
                // This must be done here because there is no guarantee of 
                // getting the location consent from a geofence call.
                if (false == permissionsChecked)
                {
                    GetGeopositionAsync();
                    permissionsChecked = true;
                }

                // get lat/long/radius, the fence name (fenceKey), 
                // and other properties from controls,
                // depending on data in controls for activation time
                // and duration the appropriate
                // constructor will be used.
                Geofence geofence = GenerateGeofence();

                // Add the geofence to the GeofenceMonitor's
                // collection of fences
                GeofenceMonitor.Current.Geofences.Add(geofence);

                // add geofence to listview
                AddGeofenceToRegisteredGeofenceListView(geofence);
            }
            catch (System.UnauthorizedAccessException)
            {
                if (DeviceAccessStatus.DeniedByUser == accessInfo.CurrentStatus)
                {
                    rootPage.NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType.StatusMessage);
                }
                else if (DeviceAccessStatus.DeniedBySystem == accessInfo.CurrentStatus)
                {
                    rootPage.NotifyUser("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", NotifyType.StatusMessage);
                }
                else if (DeviceAccessStatus.Unspecified == accessInfo.CurrentStatus)
                {
                    rootPage.NotifyUser("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", NotifyType.StatusMessage);
                }
            }
            catch (TaskCanceledException)
            {
                rootPage.NotifyUser("Canceled", NotifyType.StatusMessage);
            }
            catch (Exception ex)
            {
                // GeofenceMonitor failed in adding a geofence
                // exceptions could be from out of memory, lat/long out of range,
                // too long a name, not a unique name, specifying an activation
                // time + duration that is still in the past
                rootPage.NotifyUser(ex.ToString(), NotifyType.ErrorMessage);
            }
            finally
            {
            }
        }

        /// <summary>
        /// Helper method to invoke Geolocator.GetGeopositionAsync.
        /// </summary>
        async private void GetGeopositionAsync()
        {
            rootPage.NotifyUser("Checking permissions...", NotifyType.StatusMessage);

            inGetPositionAsync = true;

            try
            {
                // Get cancellation token
                cts = new CancellationTokenSource();
                CancellationToken token = cts.Token;

                // Carry out the operation
                await geolocator.GetGeopositionAsync().AsTask(token);
            }
            catch (UnauthorizedAccessException)
            {
                if (DeviceAccessStatus.DeniedByUser == accessInfo.CurrentStatus)
                {
                    rootPage.NotifyUser("Location has been disabled by the user. Enable access through the settings charm.", NotifyType.StatusMessage);
                }
                else if (DeviceAccessStatus.DeniedBySystem == accessInfo.CurrentStatus)
                {
                    rootPage.NotifyUser("Location has been disabled by the system. The administrator of the device must enable location access through the location control panel.", NotifyType.StatusMessage);
                }
                else if (DeviceAccessStatus.Unspecified == accessInfo.CurrentStatus)
                {
                    rootPage.NotifyUser("Location has been disabled by unspecified source. The administrator of the device may need to enable location access through the location control panel, then enable access through the settings charm.", NotifyType.StatusMessage);
                }
            }
            catch (TaskCanceledException)
            {
                rootPage.NotifyUser("Task canceled", NotifyType.StatusMessage);
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

            inGetPositionAsync = false;
        }

        private class GeofenceItemCollection : System.Collections.ObjectModel.ObservableCollection<GeofenceItem>
        {
        }

        private class GeofenceEventItemCollection : System.Collections.ObjectModel.ObservableCollection<string>
        {
        }
    }
}
