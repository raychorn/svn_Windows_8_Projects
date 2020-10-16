using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage.Streams;

using SDKTemplate;

using Windows.Devices.Bluetooth.GenericAttributeProfile;

namespace BluetoothGattHeartRate
{
    public class HeartRateMeasurement
    {
        public ushort HeartRateValue { get; set; }
        public ushort ExpendedEnergy { get; set; }
        public DateTimeOffset Timestamp { get; set; }

        public override string ToString()
        {
            return HeartRateValue.ToString() + " bpm @ " + Timestamp.ToString();
        }
    }


    public delegate void ValueChangeCompletedHandler(HeartRateMeasurement heartRateMeasurementValue);

    public class HeartRateService
    {
        // Heart Rate Constants

        // The Characteristic we want to obtain measurements for is the Heart Rate Measurement characteristic
        private Guid CHARACTERISTIC_UUID = GattCharacteristicUuids.HeartRateMeasurement;
        // Heart Rate devices typically have only one Heart Rate Measurement characteristic.
        // Make sure to check your device's documentation to find out how many characteristics your specific device has.
        private const int CHARACTERISTIC_INDEX = 0;
        // The Heart Rate Profile specification requires that the Heart Rate Measurement characteristic is notifiable.
        private const GattClientCharacteristicConfigurationDescriptorValue CHARACTERISTIC_NOTIFICATION_TYPE = 
            GattClientCharacteristicConfigurationDescriptorValue.Notify;

        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser().
        MainPage rootPage = MainPage.Current;

        private static HeartRateService instance = new HeartRateService();
        private GattDeviceService service;
        private List<HeartRateMeasurement> datapoints;

        public event ValueChangeCompletedHandler ValueChangeCompleted;

        public static HeartRateService Instance
        {
            get { return instance; }
        }

        public bool IsServiceInitialized { get; set; }

        public GattDeviceService Service
        {
            get { return service; }
        }

        public HeartRateMeasurement[] DataPoints
        {
            get
            {
                HeartRateMeasurement[] retval;
                lock (datapoints)
                {
                    retval = datapoints.ToArray();
                }

                return retval;
            }
        }

        private HeartRateService()
        {
            datapoints = new List<HeartRateMeasurement>();
            App.Current.Suspending += App_Suspending;
            App.Current.Resuming += App_Resuming;
        }

        private void App_Resuming(object sender, object e)
        {
            // Since the Windows Runtime will close resources to the device when the app is suspended,
            // the device needs to be reinitialized when the app is resumed.
        }

        private void App_Suspending(object sender, Windows.ApplicationModel.SuspendingEventArgs e)
        {
            this.IsServiceInitialized = false;

            // This is an appropriate place to save to persistent storage any datapoint the application cares about.
            // For the purpose of this sample we just discard any values.
            this.datapoints.Clear();

            // Allow the GattDeviceService to get cleaned up by the Windows Runtime.
            // The Windows runtime will clean up resources used by the GattDeviceService object when the application is
            // suspended. The GattDeviceService object will be invalid once the app resumes, which is why it must be 
            // marked as invalid, and reinitalized when the application resumes.
            if (service != null)
            {
                this.service.Dispose();
                this.service = null;
            }
        }

        public async Task InitializeServiceAsync(string deviceId)
        {
            try
            {
                service = await GattDeviceService.FromIdAsync(deviceId);
                if (service == null)
                {
                    rootPage.NotifyUser("Access to the device is denied, because the application was not granted access, " +
                        "or the device is currently in use by another application.",
                        NotifyType.StatusMessage);
                }
                else
                {
                    // The Heart Rate Profile specifies that the Heart Rate Service will contain a single 
                    // Heart Rate Measurement Characteristic.
                    var characteristic = service.GetCharacteristics(CHARACTERISTIC_UUID)[CHARACTERISTIC_INDEX];

                    // Register the event handler for receiving device notification data
                    characteristic.ValueChanged += Characteristic_ValueChanged;

                    // Set the Client Characteristic Configuration descriptor on the device, 
                    // registering for Characteristic Value Changed notifications
                    GattCommunicationStatus status =
                        await characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(
                        CHARACTERISTIC_NOTIFICATION_TYPE);

                    if (status == GattCommunicationStatus.Unreachable)
                    {
                        rootPage.NotifyUser("Your device is unreachable, most likely the device is out of range, " +
                            "or is running low on battery, please make sure your device is working and try again.",
                            NotifyType.StatusMessage);
                    }
                    else
                    {
                        IsServiceInitialized = true;
                    }
                }
            }
            catch (Exception e)
            {
                rootPage.NotifyUser("ERROR: Accessing your device failed." + Environment.NewLine + e.Message,
                    NotifyType.ErrorMessage);
            }
        }

        /// <summary>
        /// Process the raw data read from the device into an application usable string, according to the Bluetooth
        /// Specification.
        /// </summary>
        /// <param name="bodySensorLocationData">Raw data read from the heart rate monitor.</param>
        /// <returns>The textual representation of the Body Sensor Location.</returns>
        public string ProcessBodySensorLocationData(byte[] bodySensorLocationData)
        {
            // The Bluetooth Heart Rate Profile specifies that the Body Sensor Location characteristic value has
            // a single byte of data
            byte bodySensorLocationValue = bodySensorLocationData[0];
            string retval;

            retval = "";
            switch (bodySensorLocationValue)
            {
                case 0:
                    retval += "Other";
                    break;
                case 1:
                    retval += "Chest";
                    break;
                case 2:
                    retval += "Wrist";
                    break;
                case 3:
                    retval += "Finger";
                    break;
                case 4:
                    retval += "Hand";
                    break;
                case 5:
                    retval += "Ear Lobe";
                    break;
                case 6:
                    retval += "Foot";
                    break;
                default:
                    retval = "";
                    break;
            }
            return retval;
        }

        /// <summary>
        /// Process the raw data received from the device into application usable data, 
        /// according the the Bluetooth Heart Rate Profile.
        /// </summary>
        /// <param name="data">Raw data received from the heart rate monitor.</param>
        /// <returns>The heart rate measurement value.</returns>
        private HeartRateMeasurement ProcessData(byte[] data)
        {
            // Heart Rate profile defined flag values
            const byte HEART_RATE_VALUE_FORMAT = 0x01;
            const byte ENERGY_EXPANDED_STATUS = 0x08;

            byte currentOffset = 0;
            byte flags = data[currentOffset];
            bool isHeartRateValueSizeLong = ((flags & HEART_RATE_VALUE_FORMAT) != 0);
            bool hasEnergyExpended = ((flags & ENERGY_EXPANDED_STATUS) != 0);

            currentOffset++;

            ushort heartRateMeasurementValue = 0;

            if (isHeartRateValueSizeLong)
            {
                heartRateMeasurementValue = (ushort)((data[currentOffset + 1] << 8) + data[currentOffset]);
                currentOffset += 2;
            }
            else
            {
                heartRateMeasurementValue = data[currentOffset];
                currentOffset++;
            }

            ushort expendedEnergyValue = 0;

            if (hasEnergyExpended)
            {
                expendedEnergyValue = (ushort)((data[currentOffset + 1] << 8) + data[currentOffset]);
                currentOffset += 2;
            }

            // The Heart Rate Bluetooth profile can also contain sensor contact status information,
            // and R-Wave interval measurements, which can also be processed here. 
            // For the purpose of this sample, we don't need to interpret that data.

            return new HeartRateMeasurement
            {
                HeartRateValue = heartRateMeasurementValue,
                ExpendedEnergy = expendedEnergyValue
            };

        }

        /// <summary>
        /// Invoked when Windows receives data from your Bluetooth device.
        /// </summary>
        /// <param name="sender">The GattCharacteristic object whose value is received.</param>
        /// <param name="args">The new characteristic value sent by the device.</param>
        private void Characteristic_ValueChanged(
            GattCharacteristic sender,
            GattValueChangedEventArgs args)
        {
            var data = new byte[args.CharacteristicValue.Length];

            DataReader.FromBuffer(args.CharacteristicValue).ReadBytes(data);

            // Process the raw data received from the device.
            var value = ProcessData(data);
            value.Timestamp = args.Timestamp;

            lock (datapoints)
            {
                datapoints.Add(value);
            }

            if (ValueChangeCompleted != null)
            {
                ValueChangeCompleted(value);
            }
        }
    }
}
