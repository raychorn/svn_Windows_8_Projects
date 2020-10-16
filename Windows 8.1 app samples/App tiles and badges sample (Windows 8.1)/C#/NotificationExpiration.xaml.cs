//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

using NotificationsExtensions.TileContent;
using SDKTemplate;
using System;
using Windows.UI.Notifications;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Navigation;

namespace Tiles
{
    public sealed partial class NotificationExpiration : SDKTemplate.Common.LayoutAwarePage
    {
        #region TemplateCode
        MainPage rootPage = MainPage.Current;

        public NotificationExpiration()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }

        #endregion TemplateCode

        void UpdateTileExpiring_Click(object sender, RoutedEventArgs e)
        {
            int seconds;
            if (!Int32.TryParse(Time.Text, out seconds))
            {
                seconds = 10;
            }

            Windows.Globalization.Calendar cal = new Windows.Globalization.Calendar();
            cal.SetToNow();
            cal.AddSeconds(seconds);

            var longTime = new Windows.Globalization.DateTimeFormatting.DateTimeFormatter("longtime");
            DateTimeOffset expiryTime = cal.GetDateTime();
            string expiryTimeString = longTime.Format(expiryTime);

            ITileWide310x150Text04 wide310x150TileContent = TileContentFactory.CreateTileWide310x150Text04();
            wide310x150TileContent.TextBodyWrap.Text = "This notification will expire at " + expiryTimeString;

            ITileSquare150x150Text04 square150x150TileContent = TileContentFactory.CreateTileSquare150x150Text04();
            square150x150TileContent.TextBodyWrap.Text = "This notification will expire at " + expiryTimeString;
            wide310x150TileContent.Square150x150Content = square150x150TileContent;

            TileNotification tileNotification = wide310x150TileContent.CreateNotification();

            // set the expirationTime
            tileNotification.ExpirationTime = expiryTime;
            TileUpdateManager.CreateTileUpdaterForApplication().Update(tileNotification);

            OutputTextBlock.Text = "Tile notification sent. It will expire at " + expiryTimeString;
        }
    }
}