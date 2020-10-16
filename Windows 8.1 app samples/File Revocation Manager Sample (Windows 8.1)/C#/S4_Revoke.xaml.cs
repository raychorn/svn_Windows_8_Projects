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
using Windows.Security.EnterpriseData;

namespace FileRevocation
{
    /// <summary>
    /// An empty page that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class S4_Revoke : SDKTemplate.Common.LayoutAwarePage
    {
        // A pointer back to the main page.  This is needed if you want to call methods in MainPage such
        // as NotifyUser()
        MainPage RootPage = MainPage.Current;

        public S4_Revoke()
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

        /// <summary>
        /// Revoke the enterprise id that user entered
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Revoke_Click(object sender, RoutedEventArgs e)
        {
            if ("" == InputTextBox.Text)
            {
                RootPage.NotifyUser("Please enter an Enterpise ID that you want to use.", NotifyType.ErrorMessage);
                return;
            }

            FileRevocationManager.Revoke(InputTextBox.Text);

            RootPage.NotifyUser("The Enterprise ID " + InputTextBox.Text + " was revoked. The files protected by it will not be accessible anymore.", NotifyType.StatusMessage);
        }

    }
}
