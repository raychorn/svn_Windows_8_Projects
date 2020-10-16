using System;
using System.Threading;
using System.Threading.Tasks;
using System.Collections.Generic;
using System.Windows;
using Windows.ApplicationModel.Background;
using Windows.Data.Json;
using Windows.Storage;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using Windows.Devices.Geolocation;
using Windows.Devices.Geolocation.Geofencing;

namespace BackgroundTask
{
    // this is repeated from GeofenceItem and is used
    // to create a string reprentation of the geofence event fired
    public enum GeofenceReason { Add, Entered, Exited, Expired, Used };

    public sealed class LocationBackgroundTask : IBackgroundTask
    {
        CancellationTokenSource cts = null;

        async void IBackgroundTask.Run(IBackgroundTaskInstance taskInstance)
        {
            BackgroundTaskDeferral deferral = taskInstance.GetDeferral();

            try
            {
                // Associate a cancellation handler with the background task.
                taskInstance.Canceled += new BackgroundTaskCanceledEventHandler(OnCanceled);

                // Get cancellation token
                if (cts == null)
                {
                    cts = new CancellationTokenSource();
                }
                CancellationToken token = cts.Token;

                // Create geolocator object
                Geolocator geolocator = new Geolocator();

                // Make the request for the current position
                Geoposition pos = await geolocator.GetGeopositionAsync().AsTask(token);

                DateTime currentTime = DateTime.Now;

                WriteStatusToAppdata("Time: " + currentTime.ToString());
                WriteGeolocToAppdata(pos);
            }
            catch (UnauthorizedAccessException)
            {
                WriteStatusToAppdata("Disabled");
                WipeGeolocDataFromAppdata();
            }
            catch (Exception ex)
            {
                // If there are no location sensors GetGeopositionAsync()
                // will timeout -- that is acceptable.
                const int WaitTimeoutHResult = unchecked((int)0x80070102);

                if (ex.HResult == WaitTimeoutHResult) // WAIT_TIMEOUT
                {
                    WriteStatusToAppdata("An operation requiring location sensors timed out. Possibly there are no location sensors.");
                }
                else
                {
                    WriteStatusToAppdata(ex.ToString());
                }

                WipeGeolocDataFromAppdata();
            }
            finally
            {
                cts = null;

                deferral.Complete();
            }
        }

        private void WriteGeolocToAppdata(Geoposition pos)
        {
            var settings = ApplicationData.Current.LocalSettings;
            settings.Values["Latitude"] = pos.Coordinate.Point.Position.Latitude.ToString();
            settings.Values["Longitude"] = pos.Coordinate.Point.Position.Longitude.ToString();
            settings.Values["Accuracy"] = pos.Coordinate.Accuracy.ToString();
        }

        private void WipeGeolocDataFromAppdata()
        {
            var settings = ApplicationData.Current.LocalSettings;
            settings.Values["Latitude"] = "";
            settings.Values["Longitude"] = "";
            settings.Values["Accuracy"] = "";
        }

        private void WriteStatusToAppdata(string status)
        {
            var settings = ApplicationData.Current.LocalSettings;
            settings.Values["Status"] = status;
        }

        private void OnCanceled(IBackgroundTaskInstance sender, BackgroundTaskCancellationReason reason)
        {
            if (cts != null)
            {
                cts.Cancel();
                cts = null;
            }
        }
    }

    public sealed class GeofenceBackgroundTask : IBackgroundTask
    {
        CancellationTokenSource cts = null;

        async void IBackgroundTask.Run(IBackgroundTaskInstance taskInstance)
        {
            BackgroundTaskDeferral deferral = taskInstance.GetDeferral();

            try
            {
                // Associate a cancellation handler with the background task.
                taskInstance.Canceled += new BackgroundTaskCanceledEventHandler(OnCanceled);

                // Get cancellation token
                if (cts == null)
                {
                    cts = new CancellationTokenSource();
                }
                CancellationToken token = cts.Token;

                await GetGeofenceStateChangedReportsAsync(token);
            }
            catch (UnauthorizedAccessException)
            {
                WriteStatusToAppdata("Location Permissions disabled by user. Enable access through the settings charm to enable the background task.");
                WipeGeofenceDataFromAppdata();
            }
            finally
            {
                cts = null;

                deferral.Complete();
            }
        }

        private void WipeGeofenceDataFromAppdata()
        {
            var settings = ApplicationData.Current.LocalSettings;
            settings.Values["GeofenceEvent"] = "";
        }

        private void WriteStatusToAppdata(string status)
        {
            var settings = ApplicationData.Current.LocalSettings;
            settings.Values["Status"] = status;
        }

        private void OnCanceled(IBackgroundTaskInstance sender, BackgroundTaskCancellationReason reason)
        {
            if (cts != null)
            {
                cts.Cancel();
                cts = null;
            }
        }

        private Task GetGeofenceStateChangedReportsAsync(CancellationToken token)
        {
            System.Action action = () =>
            {
                JsonArray jsonArray = new JsonArray();

                GeofenceMonitor monitor = GeofenceMonitor.Current;

                Geoposition pos = monitor.LastKnownGeoposition;

                Windows.Globalization.Calendar calendar = new Windows.Globalization.Calendar();
                calendar.SetDateTime(pos.Coordinate.Timestamp);

                jsonArray.Add(JsonValue.CreateNumberValue(calendar.Year));
                jsonArray.Add(JsonValue.CreateNumberValue(calendar.Month));
                jsonArray.Add(JsonValue.CreateNumberValue(calendar.Day));
                jsonArray.Add(JsonValue.CreateNumberValue(calendar.Hour));
                jsonArray.Add(JsonValue.CreateNumberValue(calendar.Minute));
                jsonArray.Add(JsonValue.CreateNumberValue(calendar.Second));
                jsonArray.Add(JsonValue.CreateNumberValue(calendar.Period)); // 1:AM or 2:PM

                jsonArray.Add(JsonValue.CreateNumberValue(pos.Coordinate.Point.Position.Latitude));
                jsonArray.Add(JsonValue.CreateNumberValue(pos.Coordinate.Point.Position.Longitude));

                // Retrieve a vector of state change reports
                var reports = GeofenceMonitor.Current.ReadReports();

                foreach (GeofenceStateChangeReport report in reports)
                {
                    GeofenceState state = report.NewState;

                    jsonArray.Add(JsonValue.CreateStringValue(report.Geofence.Id));

                    if (state == GeofenceState.Removed)
                    {
                        GeofenceRemovalReason reason = report.RemovalReason;

                        if (reason == GeofenceRemovalReason.Expired)
                        {
                            jsonArray.Add(JsonValue.CreateNumberValue((Double)GeofenceReason.Expired));
                        }
                        else if (reason == GeofenceRemovalReason.Used)
                        {
                            jsonArray.Add(JsonValue.CreateNumberValue((Double)GeofenceReason.Used));
                        }
                    }
                    else if (state == GeofenceState.Entered)
                    {
                        jsonArray.Add(JsonValue.CreateNumberValue((Double)GeofenceReason.Entered));
                    }
                    else if (state == GeofenceState.Exited)
                    {
                        jsonArray.Add(JsonValue.CreateNumberValue((Double)GeofenceReason.Exited));
                    }
                }

                string jsonString = jsonArray.Stringify();

                var settings = ApplicationData.Current.LocalSettings;
                settings.Values["GeofenceEvent"] = jsonString;
            };

            // Construct an unstarted task
            Task task = new Task(action, token);

            task.Start();

            return task;
        }
    }
}
