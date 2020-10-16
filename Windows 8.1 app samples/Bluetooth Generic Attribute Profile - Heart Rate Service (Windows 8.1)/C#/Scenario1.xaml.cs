//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using System.Collections.Generic;
using Windows.Storage.Streams;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using SDKTemplate;
using System;

using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;

namespace BluetoothGattHeartRate
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario1 : SDKTemplate.Common.LayoutAwarePage
    {
        // A pointer back to the main page.  This is needed if you want 
        // to call methods in MainPage such as NotifyUser().
        MainPage rootPage = MainPage.Current;

        public Scenario1()
        {
            this.InitializeComponent();
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached. The Parameter property is typically
        /// used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            if (HeartRateService.Instance.IsServiceInitialized)
            {
                outputDataChart.Loaded += outputDataChart_Loaded;
                foreach (var measurement in HeartRateService.Instance.DataPoints)
                {
                    outputListBox.Items.Add(measurement.ToString());
                }
                outputGrid.Visibility = Visibility.Visible;
                RunButton.IsEnabled = false;
            }
            HeartRateService.Instance.ValueChangeCompleted += Instance_ValueChangeCompleted;
        }
        
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            HeartRateService.Instance.ValueChangeCompleted -= Instance_ValueChangeCompleted;
        }

        void outputDataChart_Loaded(object sender, RoutedEventArgs e)
        {
            outputDataChart.PlotChart(HeartRateService.Instance.DataPoints);
        }

        private async void Instance_ValueChangeCompleted(HeartRateMeasurement heartRateMeasurementValue)
        {
            // Serialize UI update to the the main UI thread.
            await this.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                statusTextBlock.Text = "Latest received heart rate measurement: " +
                    heartRateMeasurementValue.HeartRateValue;

                outputDataChart.PlotChart(HeartRateService.Instance.DataPoints);

                outputListBox.Items.Insert(0, heartRateMeasurementValue);
            });
        }

        private async void RunButton_Click(object sender, RoutedEventArgs e)
        {
            RunButton.IsEnabled = false;

            var devices = await DeviceInformation.FindAllAsync(
                GattDeviceService.GetDeviceSelectorFromUuid(GattServiceUuids.HeartRate));

            DevicesListBox.Items.Clear();

            if (devices.Count > 0)
            {
                foreach (var device in devices)
                {
                    DevicesListBox.Items.Add(device);
                }
                DevicesListBox.Visibility = Visibility.Visible;
            }
            else
            {
                rootPage.NotifyUser("Could not find any Heart Rate devices. Please make sure your device is paired " +
                    "and powered on!",
                    NotifyType.StatusMessage);
            }
            RunButton.IsEnabled = true;
        }

        private async void DevicesListBox_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            RunButton.IsEnabled = false;
            var device = DevicesListBox.SelectedItem as DeviceInformation;
            DevicesListBox.Visibility = Visibility.Collapsed;

            statusTextBlock.Text = "Initializing device...";
            await HeartRateService.Instance.InitializeServiceAsync(device.Id);
            statusTextBlock.Text = "Waiting for device to send data...";

            outputGrid.Visibility = Visibility.Visible;
            outputDataChart.PlotChart(HeartRateService.Instance.DataPoints);
        }
    }
}
