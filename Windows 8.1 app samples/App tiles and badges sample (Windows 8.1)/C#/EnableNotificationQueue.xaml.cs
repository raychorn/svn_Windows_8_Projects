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
    public sealed partial class EnableNotificationQueue : SDKTemplate.Common.LayoutAwarePage
    {
        #region TemplateCode
        MainPage rootPage = MainPage.Current;

        public EnableNotificationQueue()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }
        #endregion TemplateCode

        void ClearTile_Click(object sender, RoutedEventArgs e)
        {
            TileUpdateManager.CreateTileUpdaterForApplication().Clear();
            OutputTextBlock.Text = "Tile cleared";
        }

        void UpdateTile_Click(object sender, RoutedEventArgs e)
        {
            ITileWide310x150Text03 wide310x150TileContent = TileContentFactory.CreateTileWide310x150Text03();
            wide310x150TileContent.TextHeadingWrap.Text = TextContent.Text;

            ITileSquare150x150Text04 square150x150TileContent = TileContentFactory.CreateTileSquare150x150Text04();
            square150x150TileContent.TextBodyWrap.Text = TextContent.Text;
            wide310x150TileContent.Square150x150Content = square150x150TileContent;

            TileNotification tileNotification = wide310x150TileContent.CreateNotification();

            string tag = "TestTag01";
            if (!Id.Text.Equals(String.Empty))
            {
                tag = Id.Text;
            }

            // set the tag on the notification
            tileNotification.Tag = tag;
            TileUpdateManager.CreateTileUpdaterForApplication().Update(tileNotification);

            OutputTextBlock.Text = "Tile notification sent. It is tagged with '" + tag + "'.\n" + MainPage.PrettyPrint(square150x150TileContent.GetContent());
        }

        void EnableNotificationQueue_Click(object sender, RoutedEventArgs e)
        {
            // Enable the notification queue - this only needs to be called once in the lifetime of your app.
            TileUpdateManager.CreateTileUpdaterForApplication().EnableNotificationQueue(true);
            OutputTextBlock.Text = "Notification cycling enabled for all tile sizes.";
        }

        void DisableNotificationQueue_Click(object sender, RoutedEventArgs e)
        {
            // Disable the notification queue - this only needs to be called once in the lifetime of your app.
            TileUpdateManager.CreateTileUpdaterForApplication().EnableNotificationQueue(false);
            OutputTextBlock.Text = "Notification cycling disabled for all tile sizes.";
        }

        void EnableSquare150x150NotificationQueue_Click(object sender, RoutedEventArgs e)
        {
            // Enable the notification queue for Square150x150 Tiles.
            TileUpdateManager.CreateTileUpdaterForApplication().EnableNotificationQueueForSquare150x150(true);
            OutputTextBlock.Text = "Notification cycling enabled for Square150x150 Tiles.";
        }

        void DisableSquare150x150NotificationQueue_Click(object sender, RoutedEventArgs e)
        {
            // Disable the notification queue for Square150x150 Tiles.
            TileUpdateManager.CreateTileUpdaterForApplication().EnableNotificationQueueForSquare150x150(false);
            OutputTextBlock.Text = "Notification cycling disabled for Square150x150 Tiles.";
        }

        void EnableWide310x150NotificationQueue_Click(object sender, RoutedEventArgs e)
        {
            // Enable the notification queue for Wide310x150 Tiles.
            TileUpdateManager.CreateTileUpdaterForApplication().EnableNotificationQueueForWide310x150(true);
            OutputTextBlock.Text = "Notification cycling enabled for Wide310x150 Tiles.";
        }

        void DisableWide310x150NotificationQueue_Click(object sender, RoutedEventArgs e)
        {
            // Disable the notification queue for Wide310x150 Tiles.
            TileUpdateManager.CreateTileUpdaterForApplication().EnableNotificationQueueForWide310x150(false);
            OutputTextBlock.Text = "Notification cycling disabled for Wide310x150 Tiles.";
        }

        void EnableSquare310x310NotificationQueue_Click(object sender, RoutedEventArgs e)
        {
            // Enable the notification queue for Square310x310 Tiles.
            TileUpdateManager.CreateTileUpdaterForApplication().EnableNotificationQueueForSquare310x310(true);
            OutputTextBlock.Text = "Notification cycling enabled for Square310x310 Tiles.";
        }

        void DisableSquare310x310NotificationQueue_Click(object sender, RoutedEventArgs e)
        {
            // Disable the notification queue for Square310x310 Tiles.
            TileUpdateManager.CreateTileUpdaterForApplication().EnableNotificationQueueForSquare310x310(false);
            OutputTextBlock.Text = "Notification cycling disabled for Square310x310 Tiles.";
        }
    }
}