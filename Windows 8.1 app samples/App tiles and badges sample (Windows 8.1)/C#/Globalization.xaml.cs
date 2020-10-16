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
using Windows.ApplicationModel.Resources.Core;
using Windows.UI.Notifications;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Navigation;

namespace Tiles
{
    public sealed partial class Globalization : SDKTemplate.Common.LayoutAwarePage
    {
        #region TemplateCode
        MainPage rootPage = MainPage.Current;

        public Globalization()
        {
            this.InitializeComponent();
        }

        protected override void OnNavigatedTo(NavigationEventArgs e)
        {
        }
        #endregion TemplateCode

        void ViewCurrentResources_Click(object sender, RoutedEventArgs e)
        {
            ResourceContext defaultContextForCurrentView = ResourceContext.GetForCurrentView();

            string asls;
            defaultContextForCurrentView.QualifierValues.TryGetValue("Language", out asls);

            string scale;
            defaultContextForCurrentView.QualifierValues.TryGetValue("Scale", out scale);

            string contrast;
            defaultContextForCurrentView.QualifierValues.TryGetValue("Contrast", out contrast);

            OutputTextBlock.Text = "Your system is currently set to the following values: Application Language: " + asls + ", Scale: " + scale + ", Contrast: " + contrast + ". If using web images and addImageQuery, the following query string would be appened to the URL: ?ms-lang=" + asls + "&ms-scale=" + scale + "&ms-contrast=" + contrast;
        }

        void SendTileNotificationWithQueryStrings_Click(object sender, RoutedEventArgs e)
        {
            ITileWide310x150ImageAndText01 wide310x150TileContent = TileContentFactory.CreateTileWide310x150ImageAndText01();
            wide310x150TileContent.TextCaptionWrap.Text = "This tile notification uses query strings for the image src.";

            wide310x150TileContent.Image.Src = ImageUrl.Text;
            wide310x150TileContent.Image.Alt = "Web image";

            // enable AddImageQuery on the notification
            wide310x150TileContent.AddImageQuery = true;

            ITileSquare150x150Image square150x150TileContent = TileContentFactory.CreateTileSquare150x150Image();
            square150x150TileContent.Image.Src = ImageUrl.Text;
            square150x150TileContent.Image.Alt = "Web image";

            // include the square template.
            wide310x150TileContent.Square150x150Content = square150x150TileContent;

            // send the notification to the app's application tile
            TileUpdateManager.CreateTileUpdaterForApplication().Update(wide310x150TileContent.CreateNotification());

            OutputTextBlock.Text = MainPage.PrettyPrint(wide310x150TileContent.GetContent());
        }

        void SendTileNotificationScaledImage_Click(object sender, RoutedEventArgs e)
        {
            string scale;
            ResourceContext.GetForCurrentView().QualifierValues.TryGetValue("Scale", out scale);

            ITileWide310x150SmallImageAndText03 wide310x150TileContent = TileContentFactory.CreateTileWide310x150SmallImageAndText03();
            wide310x150TileContent.TextBodyWrap.Text = "blueWide310x150.png in the xml is actually blueWide310x150.scale-" + scale + ".png";
            wide310x150TileContent.Image.Src = "ms-appx:///images/blueWide310x150.png";
            wide310x150TileContent.Image.Alt = "Blue wide";

            ITileSquare150x150Image square150x150TileContent = TileContentFactory.CreateTileSquare150x150Image();
            square150x150TileContent.Image.Src = "ms-appx:///images/graySquare150x150.png";
            square150x150TileContent.Image.Alt = "Gray square";
            wide310x150TileContent.Square150x150Content = square150x150TileContent;

            TileUpdateManager.CreateTileUpdaterForApplication().Update(wide310x150TileContent.CreateNotification());

            OutputTextBlock.Text = MainPage.PrettyPrint(wide310x150TileContent.GetContent());
        }

        void SendTextResourceTileNotification_Click(object sender, RoutedEventArgs e)
        {
            ITileWide310x150Text03 wide310x150TileContent = TileContentFactory.CreateTileWide310x150Text03();

            // check out /en-US/resources.resw to understand where this string will come from
            wide310x150TileContent.TextHeadingWrap.Text = "ms-resource:greeting";

            ITileSquare150x150Text04 square150x150TileContent = TileContentFactory.CreateTileSquare150x150Text04();
            square150x150TileContent.TextBodyWrap.Text = "ms-resource:greeting";
            wide310x150TileContent.Square150x150Content = square150x150TileContent;

            TileUpdateManager.CreateTileUpdaterForApplication().Update(wide310x150TileContent.CreateNotification());

            OutputTextBlock.Text = MainPage.PrettyPrint(wide310x150TileContent.GetContent());
        }
    }
}