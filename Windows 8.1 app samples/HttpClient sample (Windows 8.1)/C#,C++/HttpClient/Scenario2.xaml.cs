﻿//*********************************************************
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
using System.IO;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Windows.Security.Cryptography;
using Windows.Storage.Streams;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Navigation;
using Windows.Web.Http;

namespace Microsoft.Samples.Networking.HttpClientSample
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario2 : SDKTemplate.Common.LayoutAwarePage, IDisposable
    {
        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;

        private HttpClient httpClient;
        private CancellationTokenSource cts;

        public Scenario2()
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
            Helpers.CreateHttpClient(ref httpClient);
            cts = new CancellationTokenSource();
        }

        protected override void OnNavigatedFrom(NavigationEventArgs e)
        {
            Dispose();
        }

        private async void Start_Click(object sender, RoutedEventArgs e)
        {
            Helpers.ScenarioStarted(StartButton, CancelButton, OutputField);
            rootPage.NotifyUser("In progress", NotifyType.StatusMessage);

            try
            {
                // 'AddressField' is a disabled text box, so the value is considered trusted input. When enabling the
                // text box make sure to validate user input (e.g., by catching FormatException as shown in scenario 1).
                Uri resourceAddress = new Uri(AddressField.Text);
                HttpRequestMessage request = new HttpRequestMessage(HttpMethod.Get, resourceAddress);

                // Do not buffer the response.
                HttpResponseMessage response = await httpClient.SendRequestAsync(
                    request,
                    HttpCompletionOption.ResponseHeadersRead).AsTask(cts.Token);

                OutputField.Text += Helpers.SerializeHeaders(response);

                StringBuilder responseBody = new StringBuilder();
                using (Stream responseStream = (await response.Content.ReadAsInputStreamAsync()).AsStreamForRead())
                {
                    int read = 0;
                    byte[] responseBytes = new byte[1000];
                    do
                    {
                        read = await responseStream.ReadAsync(responseBytes, 0, responseBytes.Length);

                        responseBody.AppendFormat("Bytes read from stream: {0}", read);
                        responseBody.AppendLine();

                        // Use the buffer contents for something. We can't safely display it as a string though, since encodings
                        // like UTF-8 and UTF-16 have a variable number of bytes per character and so the last bytes in the buffer
                        // may not contain a whole character. Instead, we'll convert the bytes to hex and display the result.
                        IBuffer responseBuffer = CryptographicBuffer.CreateFromByteArray(responseBytes);
                        responseBuffer.Length = (uint)read;
                        responseBody.AppendFormat(CryptographicBuffer.EncodeToHexString(responseBuffer));
                        responseBody.AppendLine();
                    } while (read != 0);
                }
                OutputField.Text += responseBody.ToString();

                rootPage.NotifyUser("Completed", NotifyType.StatusMessage);
            }
            catch (TaskCanceledException)
            {
                rootPage.NotifyUser("Request canceled.", NotifyType.ErrorMessage);
            }
            catch (Exception ex)
            {
                rootPage.NotifyUser("Error: " + ex.Message, NotifyType.ErrorMessage);
            }
            finally
            {
                Helpers.ScenarioCompleted(StartButton, CancelButton);
            }
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            cts.Cancel();
            cts.Dispose();

            // Re-create the CancellationTokenSource.
            cts = new CancellationTokenSource();
        }

        public void Dispose()
        {
            if (httpClient != null)
            {
                httpClient.Dispose();
                httpClient = null;
            }

            if (cts != null)
            {
                cts.Dispose();
                cts = null;
            }
        }
    }
}
