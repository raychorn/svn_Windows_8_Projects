//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

using System;
using Windows.ApplicationModel.DataTransfer;
using Windows.UI.Xaml.Controls;

namespace ShareSource
{
    public sealed partial class ShareHtml : SDKTemplate.Common.SharePage
    {
        public ShareHtml()
        {
            this.InitializeComponent();
            ShareWebView.Navigate(new Uri("http://msdn.microsoft.com"));
        }

        private void ShareWebView_NavigationCompleted(object sender, WebViewNavigationCompletedEventArgs e)
        {
            ShareWebView.Visibility = Windows.UI.Xaml.Visibility.Visible;
            BlockingRect.Visibility = Windows.UI.Xaml.Visibility.Collapsed;
            LoadingProgressRing.IsActive = false;
        }

        protected override bool GetShareContent(DataRequest request)
        {
            bool succeeded = false;

            // Get the user's selection from the WebView.
            DataPackage requestData = ShareWebView.CaptureSelectedContentToDataPackageAsync().GetResults();
            DataPackageView dataPackageView = requestData.GetView();

            if ((dataPackageView != null) && (dataPackageView.AvailableFormats.Count > 0))
            {
                requestData.Properties.Title = "A web snippet for you";
                requestData.Properties.Description = "HTML selection from a WebView control"; // The description is optional.
                requestData.Properties.ContentSourceApplicationLink = ApplicationLink;
                requestData.Properties.ContentSourceWebLink = new Uri("http://msdn.microsoft.com");
                request.Data = requestData;
                succeeded = true;
            }
            else
            {
                request.FailWithDisplayText("Make a selection in the WebView control and try again.");
            }
            return succeeded;
        }
    }
}