//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
//
//*********************************************************

using Windows.Storage.Streams;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;
using SDKTemplate;
using System;


using Windows.Devices.Bluetooth.GenericAttributeProfile;

namespace BluetoothGattHeartRate
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario3 : SDKTemplate.Common.LayoutAwarePage
    {
        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;

        public Scenario3()
        {
            this.InitializeComponent();
        }

        /// <summary>
        /// Invoked when this page is about to be displayed in a Frame.
        /// </summary>
        /// <param name="e">Event data that describes how this page was reached.  The Parameter
        /// property is typically used to configure the page.</param>
        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
            if (HeartRateService.Instance.IsServiceInitialized)
            {
                HeartRateService.Instance.ValueChangeCompleted += Instance_ValueChangeCompleted;
                WriteCharacteristicValueButton.IsEnabled = true;
            }
            else
            {
                rootPage.NotifyUser("The Heart Rate Service is not initialized, please go to Scenario 1 to initialize " +
                    "the service before writing a Characteristic Value.", NotifyType.StatusMessage);
            }
        }
        
        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            HeartRateService.Instance.ValueChangeCompleted -= Instance_ValueChangeCompleted;
        }

        private async void Instance_ValueChangeCompleted(HeartRateMeasurement heartRateMeasurementValue)
        {
            // Serialize UI update to the the main UI thread.
            await this.Dispatcher.RunAsync(Windows.UI.Core.CoreDispatcherPriority.Normal, () =>
            {
                ExpendedEnergyTextBlock.Text = "Expended Energy: " + heartRateMeasurementValue.ExpendedEnergy.ToString() + " kJ";
            });
        }

        private async void WriteCharacteristicValue_Click(object sender, RoutedEventArgs e)
        {
            WriteCharacteristicValueButton.IsEnabled = false;
            try
            {
                var heartRateControlPointCharacteristics =
                    HeartRateService.Instance.Service.GetCharacteristics(GattCharacteristicUuids.HeartRateControlPoint);

                if (heartRateControlPointCharacteristics.Count > 0)
                {
                    DataWriter writer = new DataWriter();
                    writer.WriteByte(1);

                    GattCommunicationStatus status = await heartRateControlPointCharacteristics[0].WriteValueAsync(writer.DetachBuffer());

                    if (status == GattCommunicationStatus.Success)
                    {
                        rootPage.NotifyUser("Expended Energy successfully reset.", NotifyType.StatusMessage);
                    }
                    else
                    {
                        rootPage.NotifyUser("Your device is unreachable, most likely the device is out of range, " +
                            "or is running low on battery, please make sure your device is working and try again.",
                            NotifyType.StatusMessage);
                    }
                }
            }
            catch (Exception exc)
            {
                rootPage.NotifyUser("Error: " + exc.ToString(), NotifyType.ErrorMessage);
            }
            WriteCharacteristicValueButton.IsEnabled = true;
        }
    }
}
