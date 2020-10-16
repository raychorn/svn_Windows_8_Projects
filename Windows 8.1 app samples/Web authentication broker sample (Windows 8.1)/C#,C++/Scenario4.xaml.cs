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
using System.Linq;
using System.Collections.Generic;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.Graphics.Display;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.Security.Authentication.Web;
using Windows.Security.Cryptography.Core;
using Windows.Security.Cryptography;
using Windows.Storage.Streams;
using System.Text;
using System.IO;
using System.Threading.Tasks;


namespace WebAuthentication
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class Scenario4 : SDKTemplate.Common.LayoutAwarePage
    {
        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage rootPage = MainPage.Current;

        public Scenario4()
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
        }


        private void DebugPrint(String Trace)
        {
            TextBox GoogleDebugArea = rootPage.FindName("GoogleDebugArea") as TextBox;
            GoogleDebugArea.Text += Trace + "\r\n";
        }

        private void OutputToken(String TokenUri)
        {
            TextBox GoogleReturnedToken = rootPage.FindName("GoogleReturnedToken") as TextBox;
            GoogleReturnedToken.Text = TokenUri;
        }

        private async void Launch_Click(object sender, RoutedEventArgs e)
        {
            if (GoogleClientID.Text == "")
            {
                rootPage.NotifyUser("Please enter an Client ID.", NotifyType.StatusMessage);
            }
            else if (GoogleCallbackUrl.Text == "")
            {
                rootPage.NotifyUser("Please enter an Callback URL.", NotifyType.StatusMessage);
            }

            try
            {
                String GoogleURL = "https://accounts.google.com/o/oauth2/auth?client_id=" + Uri.EscapeDataString(GoogleClientID.Text) + "&redirect_uri=" + Uri.EscapeDataString(GoogleCallbackUrl.Text) + "&response_type=code&scope=" + Uri.EscapeDataString("http://picasaweb.google.com/data");

                System.Uri StartUri = new Uri(GoogleURL);
                // When using the desktop flow, the success code is displayed in the html title of this end uri
                System.Uri EndUri = new Uri("https://accounts.google.com/o/oauth2/approval?");

                DebugPrint("Navigating to: " + GoogleURL);

                WebAuthenticationResult WebAuthenticationResult = await WebAuthenticationBroker.AuthenticateAsync(
                                                        WebAuthenticationOptions.UseTitle,
                                                        StartUri,
                                                        EndUri);
                if (WebAuthenticationResult.ResponseStatus == WebAuthenticationStatus.Success)
                {
                    OutputToken(WebAuthenticationResult.ResponseData.ToString());
                }
                else if (WebAuthenticationResult.ResponseStatus == WebAuthenticationStatus.ErrorHttp)
                {
                    OutputToken("HTTP Error returned by AuthenticateAsync() : " + WebAuthenticationResult.ResponseErrorDetail.ToString());
                }
                else
                {
                    OutputToken("Error returned by AuthenticateAsync() : " + WebAuthenticationResult.ResponseStatus.ToString());
                }
            }
            catch (Exception Error)
            {
                //
                // Bad Parameter, SSL/TLS Errors and Network Unavailable errors are to be handled here.
                //
                DebugPrint(Error.ToString());
            }
        }		        


    }
}
